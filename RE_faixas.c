///Ressonância Estocástica de Limear: representação visual
///
///Para cada um dos três valores de "treshold"/limiar definidos abaixo, esse programa gerará um numero "QUADROS"
///de imagens ".pgm". Essas imagens terão oito faixas cada. A primeira faixa de cada uma será representativa do
///sinal ideal, e não será filtrada nem receberá ruído. Depois, cada faixa terá uma das amplitudes A0 a A6
///abaixo e receberá um ruído gaussiano branco (média 0 e descorrelacionado), de desvio padrão dado pela
///constante global estática "stdDev". As imagens serão nomeadas de acordo com o treshold e o numero do quadro
///A idéia é que as imagens para cada treshold sejam utilizadas para montar gifs ou assemelhados (a variação
///temporal é importante para o efeito estudado), facilitando o estudo do efeito de diferentes níveis de
///ruído na percepção visual humana para diferentes amplitudes de um sinal matematicamente bem definido.
///
///(O GIMP consegue abrir arquivos .pgm, aprox 229kb por quadro. Dá pra abrir um e arrastar os outros quadros pra
///dentro do GMP e mandar exportar em .gif, como animação, com 20 ms entre os quadros e um frame por layer. O GIF
///fica com um tamanho por volta de 15% do tamanho total dos quadros originais (ou seja, uns 1,75mb/segundo)

///apenas definicoes de parametros de teste e constantes
#define QUADROS 50 //quadros POR COMBINACAO de parametros
#define ALTURA_FAIXA 40 //havera 8 faixas por imagem, cada uma com essa altura, mais 10 px entre cada uma
#define LARGURA 600 //largura de cada faixa e da imagem como um todo
//desvios padroes de ruido branco gaussiano predefinidos
#define STD_DEV00 28
#define STD_DEV0 48
#define STD_DEV1 58
#define STD_DEV2 78
#define STD_DEV3 108
#define STD_DEV4 128
#define STD_DEV5 156
#define STD_DEV6 192
#define STD_DEV7 248
#define STD_DEV8 312
#define STD_DEV9 350
//limiares de transmissao predefinidos
#define TRESHOLD0 128
#define TRESHOLD1 150
#define TRESHOLD2 192
//amplitudes de sinal predefinidas
#define A0 128
#define A6 108
#define A5 88
#define A4 78
#define A3 58
#define A2 48
#define A1 28
//um fator de soma ao sinal, consegui melhores resultados sem usar. CUIDADDO: Amplitude + Base <= 255
#define BASE 0

#define PI 3.1416

#include <random>
#include <windows.h> //para criar pastas com os nomes bonitinhos pra cada caso

///TROCAR >AQUI< o desvio padrão do ruído! (manter o cast pra float)
static const float stdDev = (float)STD_DEV9;

///TROCAR >AQUI< o limear!
static const uint8_t treshold = TRESHOLD1;

///para gerar os números aleatórios em dist gaussiana
static std::random_device randomDevice;
static std::mt19937 randomGen(randomDevice());
static std::normal_distribution<float> normalDistribution((float)0, stdDev);
//float mesmo pq efetivamente estamos apenas interessados em inteiros entre -512 e +512


inline int GetNormDistrInt()
{
    return (int)round(normalDistribution(randomGen));
}

///Classe para gerar e gravar as Imagens, com 8 faixas de sinal cada, com 10 px entre cada faixa. Contém:
///array de pixels (1 uint8/pixel greyscale); construtores; ruído; gravação .pgm; e info de tamanho de imagem;
//usa "sinal = base + amp*(1-sen(pi/x))", x sendo uma posicao normalizada (0<=x<=0.95) e da direita p/ esquerda.
//(o artigo dizia que usava base + amp*sen(1/x), mas as curvas que eles mostravam pareciam mais com essa, sei lá)
class Image{

    public:
        //ponteiro para o array de pixels. Imagem será gravada em .pgm, tons de cinza: um byte por pixel
        uint8_t* img;

        //Construtor cria imagem sem ruído das 8 faixas:
        Image(int linhas, int colunas, uint8_t b)
        {
            this->linhas = linhas; this->colunas = colunas;
            this->pixels = colunas*(linhas*8+70); //8 pelas faixas, 70 pelos 7 espacos intermediarios de 10px
            img = (uint8_t*)malloc(pixels*sizeof(uint8_t));

            uint8_t amp = A0; //faixas seguem Amps predefinidas. Primeira "cheia", para referencia
            for(int j = 0; j < 8; j++) //cada j, uma faixa
            {
                int jaForam = ((linhas+10)*colunas)*j; //numero de pixels ja esritos
                if (j == 1) amp = A1;
                else if (j == 2) amp = A2;
                else if (j == 3) amp = A3;
                else if (j == 4) amp = A4;
                else if (j == 5) amp = A5;
                else if (j == 6) amp = A6;
                else if (j == 7) amp = A0;

                for(int i = 0; i < linhas*colunas; i++) //cada i, um pixel da faixa j (ou do espaco entre-faixa)
                {
                    int coluna_do_pixel = i%colunas;
                    float x = 1-0.95*(colunas - 1 - coluna_do_pixel)/(colunas - 1); //explicado acima classe
                    int val = b+round(amp*(1-sin(PI/(1-x)))); //explicado acima classe
                    if (val < 0) img[jaForam + i] = 0;
                    else if (val > 255) img[jaForam + i] = 255;
                    else img[jaForam + i] = (uint8_t)val;
                }

                jaForam += linhas*colunas;

                if(j!=7) for(int i = 0; i < 10*colunas; i++) img[jaForam + i] = 0; //não precisa espaço no fim
            }
        }

        Image(Image* original) //constrói cópia COM RUÍDO. Preserva a original para outras realizações de ruído
        {
            this->linhas = original->linhas; this->colunas = original->colunas; this->pixels = original->pixels;

            img = (uint8_t*)malloc(pixels*sizeof(uint8_t));

            memcpy(img, original->img, pixels*sizeof(uint8_t)); //aqui copia de fato
            //e daí adiciona ruído:

            for(int j = 1; j < 8; j++) //"arruidar" as faixa com cada Amplitude, mas não espaço entre elas
            {//nem a primeira (usada de referência), por isso j inicia em 1
                int jaForam = ((linhas+10)*colunas)*j; //número de pixels já esritos (pulando faixas intermed)
                for(int i = 0; i < linhas*colunas; i++)
                {
                    int val = GetNormDistrInt() + img[jaForam + i]; //"soma" ruído de média zero ao pixel
                    if (val < 0) img[jaForam + i] = 0;
                    else if (val > 255) img[jaForam + i] = 255;
                    else img[jaForam + i] = val;
                }
            }
        }

        //salva imagem em .pgm, em diretório próprio e ordenado com infos de parâmetros e numero de frame
        void saveTreshPGM(uint8_t treshold, int fileNumber)
        {
            std::string altura =  std::to_string( 8*(this->linhas)+70 );
            std::string largura =  std::to_string(this->colunas);
            std::string header = "P5 "+largura+" "+altura+" "+"255\n"; //pelas especificações p/ .pgm binário

            //construir nome do arquivo:
            std::string fileName = std::to_string(treshold) + "_";
            if (fileNumber < 10) fileName += "000" + std::to_string(fileNumber);
            else if (fileNumber < 100) fileName += "00" + std::to_string(fileNumber);
            else if (fileNumber < 1000) fileName += "0" + std::to_string(fileNumber);
            else if (fileNumber < 10000) fileName += std::to_string(fileNumber);
            fileName += ".pgm";

            FILE* fp;
            fp = fopen(fileName.c_str(),"wb"); //a tradição manda checar retorno, mas fica pra depois :P

            fwrite(header.c_str(),header.size(),1,fp); //coloca header .pgm no arquivo

            uint8_t on = 255; //se sinal passar do limear, grava 255 = branco
            uint8_t off = 0; //se nao, grava 0 = preto

            for(int i = 0; i < this->pixels; i++){
                if(i < (this->linhas*this->colunas)) fwrite(&img[i],1,1,fp); //primeira faixa: normal em cinza
                else if ((int)img[i] >= treshold) fwrite(&on,1,1,fp); //outras soh "on ou off"
                else fwrite(&off,1,1,fp);
            }

            fclose(fp);
        }

    private:
        int linhas, colunas, pixels;
};

int main(void){

    //cria a imagem original sem ruído, a ser copiada a cada realização do ruído:
    Image* imBase = new Image(ALTURA_FAIXA,LARGURA,BASE);

    for(int i = 0; i < QUADROS; i++){
        Image* imFinal = new Image(imBase); //construtor de cópia já adiciona o ruído
        imFinal->saveTreshPGM(treshold,i); //salva o quadro, implementando o limear
    }
}
