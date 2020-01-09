///Ressonância Estocástica de Limear: representação visual basica
///
///A ideia aqui é ilustrar um sistema sob ressonância estocástica de limear. Esse programa cria um gif para isso,
///com: 1) um SinalOriginal, desenhado mais escuro; 2) uma barra tracejada, acima do sinal, representando o
///limear; 3) linhas em cinz-claro, se extendendo a partir do sinal até a "altura" para qual aquele ponto (x) foi
///levado pela realização atual do ruído gaussiano branco; 4) uma seqência de linhas escuras verticais, mais acima,
///indicando, para cada "x", se houve cruzamento do limear ou não (informação binária), para cada realização;
///5) mais acima, uma barra mostrando a informação acumulada de quantos cruzamentos houveram para cada "x" em
///proporção ao total de realizações.
///Vários Quadros serão gravados em .pgm (por simplicidade) para que depois se monte um gif (no gimp, por exemplo)

///Sugere-se ler primeiro as definições de parâmetros, depois a main, e por último a Classe Image e seus métodos.

///definicoes de parâmetros de teste e constantes
#define MODE_LINHAS 1 //0 desliga. Nesse modo cada realização adiciona uma linha na faixa de reconstrução
#define QUADROS 200 //10/segundo dá gif razoavelmente "lível"
#define ALTURA_FAIXAS 40 //altura das faixas de sinal reconstruído e barras de sinalização de detecção
#define LARGURA 600 //largura da imagem inteira
#define STD_DEV 75 //desvio padrão do ruido branco gaussiano
#define TRESHOLD 150 //limiar de transmissao
#define AMPLITUDE 75 //amplitude do sinal
#define BASE 0 //um fator de soma ao sinal, preferi não usar. CUIDADDO: Amplitude + Base <= 255

#define PI 3.1416

#include <random>
#include <string.h>

static const float stdDev = (float)STD_DEV;

///para gerar os números aleatórios em dist gaussiana
static std::random_device randomDevice;
static std::mt19937 randomGen(randomDevice());
static std::normal_distribution<float> normalDistribution((float)0, stdDev);
//float mesmo pq efetivamente estamos apenas interessados em inteiros entre -512 e +512


inline int GetNormDistrInt()
{
    return (int)round(normalDistribution(randomGen));
}

///Gera e grava as imagens com sinal, desvio pelo ruído, limiar, status dedetecção, e reconstruções.
///Contém: array de pixels (1 uint8/pixel greyscale); array sinal+ruído; array numero-passagens-por-x;
///construtores; adição de ruído; gravação .pgm; e info de tamanho de imagem;
//usa "sinal = base + amp*(1-sen(pi/x))", x sendo uma posicao normalizada (0<=x<=0.95) e da direita p/ esquerda.
class Image{

    public:


        static uint8_t qtdePassagensLimear[];
        static uint8_t* linhasRestauracao; //usado apenas se MODE_LINHAS == 1;
        static int realizacoes;
        //ponteiro para o array de pixels. Imagem será gravada em .pgm, tons de cinza: um byte por pixel:
        uint8_t* img; ///!!VOU CONSIDERAR 0,0 COMO NA ESQUERDA E >EMBAIXO<, aumentando p/ direita e pra cima!!
        uint8_t* sinal;
        uint8_t* sinalMaisRuido;

        //Construtor cria imagem apenas com informações estáticas (sinal, limear, divisor, tamanho):
        Image(int linhas, int colunas, uint8_t b)
        {
            this->linhas = linhas; this->colunas = colunas;
            this->pixels = colunas*linhas;
            img = (uint8_t*)malloc(pixels*sizeof(uint8_t));
            sinal = (uint8_t*)malloc(colunas*sizeof(uint8_t));

            //inicializa imagem em branco (=255)
            for(int i = 0; i < linhas; i++){
                for(int j = 0; j < colunas; j++){
                    img[colunas*i + j] = 255; //cada linha tem "colunas" colunas, e linha "1" tem 1 linha já abaixo
                }
            }

            uint8_t amp = AMPLITUDE;

            //calcula o valor do sinal para cada x e o imprime:
            for(int i = 0; i < colunas; i++)
            {
                float x = 1-0.95*(colunas - 1 - i)/(colunas - 1); //explicado acima classe
                float val = b+(amp*(1-sin(PI/(1-x)))); //explicado acima classe
                if (val < 0) val = 0;
                else if (val > 255) val = 255;

                //para gravar com um "antialias":
                //primeiro, decidir os valores dos três pontos que serão usados:
                uint8_t valUp = (uint8_t)round(255*(ceil(val) - val)); //quanto mais perto do ceil, mais escuro
                uint8_t valDown = (uint8_t)round(255*(val - floor(val))); //o mesmo mas para baixo
                uint8_t posMid = (uint8_t)floor(val);

                sinal[i] = posMid;

                //depois os gravar na matriz:
                if (posMid<255) img[colunas*(posMid+1) + i] = valUp;
                img[colunas*posMid + i] = 0; //pixel central preto, em pos "colunas_por_linha*num_da_linha + coluna"
                if (posMid>0) img[colunas*(posMid-1) + i] = valDown;

                //já coloca tracejado do limiar e linha de separação do "detector":
                if ((i/10)%2 == 0) img[colunas*(TRESHOLD)+i] = 0; //(i/10)%2 -> apenas em DEZENAS pares
                img[colunas*(270)+i] = 0;
                //e uma faixa preta em cima, onde vai se reconstruir o sinal:
                drawRectang(i,270+40+10,40,1,0);
            }
        }

        //construtor de cópia adiciona infos dinâmicas a cada realização
        Image(Image* original) //constrói cópia COM RUÍDO. Preserva a original para outras realizações de ruído
        {
            this->linhas = original->linhas; this->colunas = original->colunas; this->pixels = original->pixels;

            img = (uint8_t*)malloc(pixels*sizeof(uint8_t));
            sinalMaisRuido = (uint8_t*)malloc(colunas*sizeof(uint8_t));

            memcpy(img, original->img, pixels*sizeof(uint8_t)); //aqui copia a imagem base
            memcpy(sinalMaisRuido, original->sinal, colunas*sizeof(uint8_t)); //copia sinal pra adicionar ruido
            //e daí adiciona ruído:

            for(int i = 0; i < colunas; i++)
            {
                ///cuidado: "sinalMaisRuido" começa apenas com o sinal, só ao fim se adiciona mesmo o ruído
                int val = GetNormDistrInt() + sinalMaisRuido[i]; //"soma" ruído de média zero
                if (val < 0) val = 0;
                else if (val > 255) val = 255;
                int delta = abs(val - sinalMaisRuido[i]);

                if(val>sinalMaisRuido[i]) drawRectang(i,sinalMaisRuido[i]+1,delta,1,200);
                else if(val<sinalMaisRuido[i]) drawRectang(i,sinalMaisRuido[i]-delta,delta,1,200);

                sinalMaisRuido[i] = val; //agora sim ele tem memso i sinal e o ruído

                int linhaRealizacao = realizacoes%(ALTURA_FAIXAS-1); //para MODE_LINHAS != 0
                //se passou do limear, desenha barrinha de "passou":
                if (sinalMaisRuido[i] > TRESHOLD)
                {
                    drawRectang(i,270,40,1,0);
                    if (MODE_LINHAS == 0) qtdePassagensLimear[i]++;
                    else //vai ir imprimindo de baixo para cima cada vez uma linha de 1 px com ptos que passaram:
                        linhasRestauracao[colunas*linhaRealizacao + i] = 255;
                }
                else if (MODE_LINHAS != 0) linhasRestauracao[colunas*linhaRealizacao + i] = 0;
            }
            realizacoes++;

            //por fim, desenha a barra superior "reconstruindo" o sinal:
            for(int i = 0; i < colunas; i++)
            {
                if (MODE_LINHAS == 0){
                    int val = (int)round(255*(qtdePassagensLimear[i]/(float)realizacoes));
                    drawRectang(i,256+14+40+10,40,1,val);
                }
                else {
                    for (int j = 0; j < ALTURA_FAIXAS; j++){
                        int linhaRealizacao = j%(ALTURA_FAIXAS-1);
                        drawRectang(i,256+14+40+10+linhaRealizacao,1,1,linhasRestauracao[linhaRealizacao*colunas+i]);
                    }
                }
            }
        }

        //salva imagem em .pgm, em diretório próprio e ordenado com infos de parâmetros e numero de frame
        //lembrando: 0,0: EMBAIXO na esquerda na img, mas EM CIMA na esquerda pro pgm
        void savePGM(int fileNumber)
        {
            std::string altura =  std::to_string(this->linhas);
            std::string largura =  std::to_string(this->colunas);
            std::string header = "P5 "+largura+" "+altura+" "+"255\n"; //pelas especificações p/ .pgm binário

            //construir nome do arquivo:
            std::string fileName;
            if (fileNumber < 10) fileName += "000" + std::to_string(fileNumber);
            else if (fileNumber < 100) fileName += "00" + std::to_string(fileNumber);
            else if (fileNumber < 1000) fileName += "0" + std::to_string(fileNumber);
            else if (fileNumber < 10000) fileName += std::to_string(fileNumber);
            fileName += ".pgm";

            FILE* fp;
            fp = fopen(fileName.c_str(),"wb"); //a tradição manda checar retorno, mas fica pra depois :P

            fwrite(header.c_str(),header.size(),1,fp); //coloca header .pgm no arquivo

            for(int i = linhas-1; i >= 0; i--){ //assim pela diferença na ordem vertical
                for(int j = 0; j < colunas; j++){
                    int pixel = i*colunas +j;
                    fwrite(&img[pixel],1,1,fp);
                }
            }

            fclose(fp);
        }


    private:
        int linhas, colunas, pixels;

        void drawRectang(int x0,int y0, int altura, int largura, uint8_t cinza)
        {
            for(int i = 0; i < altura; i++)
            {
                for (int j = 0; j < largura; j++)
                {
                    this->img[this->colunas*(y0+i) + x0 + j] = cinza;
                }
            }
        }
};
uint8_t Image::qtdePassagensLimear[LARGURA];
uint8_t* Image::linhasRestauracao;
int Image::realizacoes = 0;

int main(void){

    for(int i = 0; i < LARGURA; i++) Image::qtdePassagensLimear[i] = 0;

    //criamos a imagem original, a ser copiada a cada realização do ruído:
    int alturaTotal = 2*ALTURA_FAIXAS + 256+14+10+10; //duas faixas, 256 px para sinal e ruído, + espaçamentos
    Image* imBase = new Image(alturaTotal,LARGURA,BASE);
    imBase->savePGM(0);

    Image::linhasRestauracao = (uint8_t*)calloc(alturaTotal*LARGURA,sizeof(uint8_t));


    for(int j = 1; j <= QUADROS; j++){
        Image* imFinalQuadro = new Image(imBase); //construtor de cópia já adiciona as infos dinâmicas
        imFinalQuadro->savePGM(j);
    }
}
