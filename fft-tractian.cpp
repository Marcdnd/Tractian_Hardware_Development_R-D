/*
 Este programa depende do C++17 e libboost versão minima 1.50.0
 Author: Marcio Denadai - 2021-07-19
 Desafio Tractian - "[Hardware] Hardware Development R&D"
*/

#include <iostream>
#include <fstream>
#include <filesystem> // -std=c++17
#include <vector>
#include <ctime>
#include <iterator>
#include <string>
#include <algorithm>
#include <complex>
#include <boost/algorithm/string.hpp> // minimal libboost release 1.50.0
#include <boost/format.hpp> // minimal libboost release 1.29.0 

using cd = std::complex<long double>;
const long double PI = acos(-1);
// ou
//const long double PI = 4*atan(1);

/*  A class to read data from a csv file. */
class CSVReader
{
    std::string fileName;
    std::string delimeter;
public:
    CSVReader(std::string filename, std::string delm = ",") :
            fileName(filename), delimeter(delm)
    { }
    // Function to fetch data from a CSV File
    std::vector<std::vector<std::string> > getData();
};

/*
* Parses through csv file line by line and returns the data
* in vector of vector of strings.
*/
std::vector<std::vector<std::string>> CSVReader::getData()
{
    // Variáveis
    std::ifstream file(fileName);
    std::vector<std::vector<std::string>> dataList;
    std::string line = "";

    // Iterate through each line and split the content using delimeter
    while (getline(file, line))
    {
        std::vector<std::string> vec;
//        boost::replace_all(line, ",", "."); // Troca o seprador de decimais de virgula para ponto
        boost::algorithm::split(vec, line, boost::is_any_of(delimeter)); // Função identica ao Python -> "".split(',')
        dataList.push_back(vec);
    }
    // Close the File
    file.close();
    return dataList; 
}

/*
long double deg2rad(long double degree) {
    return degree * (PI/180);
}
*/


/*
Fonte desta função: https://cp-algorithms.com/algebra/fft.html#toc-tgt-0
Este algoritmo utiliza recursividade, portanto necessita de q a qtde de resgistros seja multiplo de 2^N (2,4,8,64,128,256,512 ...)
*/
void fft(std::vector<cd> & a, bool invert = false) {
    int n = a.size();

    if (n == 1)
        return;

    std::vector<cd> a0(n / 2), a1(n / 2);
    for (int i = 0; 2 * i < n; i++) {
        a0[i] = a[2*i];
        a1[i] = a[2*i+1];
    }
    fft(a0, invert);
    fft(a1, invert);

    double ang = 2 * PI / n * (invert ? -1 : 1);
    cd w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; i++) {
        a[i] = a0[i] + w * a1[i];
        a[i + n/2] = a0[i] - w * a1[i];
        if (invert) {
            a[i] /= 2;
            a[i + n/2] /= 2;
        }
        w *= wn;
    }
}

/*
Fonte desta função: https://cp-algorithms.com/algebra/fft.html#toc-tgt-0
Este algoritmo não utiliza recursividade, portanto não necessita de q a qtde de resgistro seja multiplo de 2^N
Utiliza a class complex da C++ STL
*/
void fft2(std::vector<cd> & a, bool invert = false) {
    int n = a.size();

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;

        if (i < j)
            swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i+j], v = a[i+j+len/2] * w;
                a[i+j] = u + v;
                a[i+j+len/2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert) {
        for (cd & x : a)
            x /= n;
    }
}

/*
Rotina para imprimir o vetor na tela
*/
void print_vector(std::vector<long double> vetor) {
   for(long double i : vetor) {
	std::cout << std::to_string(i) << ";";
    }
}


// Programa Principal
int main(int argc, char *argv[])
{
    // Declaração das variáveis
    std::vector<long double> x,y,z,time,total;
    std::string ArquivoCSV;
    double time_total, N, P, F;

    // Avalia se o primeiro parametro é um arquivo se não for seleciona o arquivo padrão
//    ArquivoCSV = "example-1.csv"; // Nome do arquivo padrão caso o primeiro argumento não seja passado ou o arquivo passado nele não exista
    ArquivoCSV = "1602245833-2715-NAO7856.csv";
    if(argc==2) {
        if(std::filesystem::exists(argv[1])) { // Para compilar std::filesystem::exists precisa ter C++17 -> g++ -std=c++17 ...
	    ArquivoCSV = argv[1];
        }
    }

//    CSVReader reader(ArquivoCSV,";"); // Utilize esta linha caso o arquivo CSV seja separado por ponto e virgula e comente a linha seguinte
    CSVReader reader(ArquivoCSV);

    // Carrega os dados no Get the data from CSV File
    // *TODO: Implementar rotina para detectar/ignorar cabeçalho em arquivos CSV
    std::vector<std::vector<std::string>> dataList = reader.getData();
    // Print the content of row by row on screen

    long int contador=1;
    double time_acc = 0.0;

    // *TODO: Pegar as infomacoes do arquivo para Tempo de duração (time_total), Qtde de amostras (N)
    time_total = 2.715; // 2715 milisegundos
    N = (double)(dataList.size());
    F = N / time_total; // Frquencia do sampling (amostragem)
    P = 1 / F; // Periodo da amostragem

    std::time_t epoch = 1602245833; // *TODO: pegar essa informação do arquivo
    // Imprime informações da amostragem
    std::cout << "\n********************************************************************************************\n";
    std::cout << boost::format{"Data da coleta: %2% - %1%"} % std::asctime(std::localtime(&epoch)) % epoch << std::endl;
    std::cout << "Qtde de registros no arquivo " << ArquivoCSV << ": " << dataList.size() << std::endl;
    std::cout << boost::format{"Time: %1%(s)\tN: %2%\tF: %3%(Hz)\tP: %4%(s)\n"} % time_total % N % F % P;

    // Loop para carregar e converter os valores do CSV nos vetores respectivos
    const int NUMF=3; // Numero de campos do registro

    for(std::vector<std::string> vec : dataList) // for vec in datalist: ...
    {
	if (vec.size()==NUMF) {
//            boost::replace_all(vec[0],",","."); // Troca virvula por ponto para separaçõa dos decimais
       	    x.push_back(std::stold(vec[0]));
       	    y.push_back(std::stold(vec[1]));
       	    z.push_back(std::stold(vec[2]));

	    time_acc += P;
	    time.push_back(time_acc);

            total.push_back( std::stold(vec[0]) + std::stold(vec[1]) + std::stold(vec[2]) );

	} else {
            if (vec.size()>NUMF) {
	        std::cout << std::endl << "ERRO - Excesso de Campos:\tVerifique a linha " << contador << " do arquivo CSV.";
            } else {
	        std::cout << std::endl << "ERRO - Insuficiencia de Campos:\tVerifique a linha " << contador << " do arquivo CSV.";
	    }
	}
        ++contador;
    }

/*  // Descomentar este bloco caso queira "depurar" os vetores carregados

    std::cout << "\n\nVetor X (" << x.size() << " registros)\n";
    print_vector(x);

    std::cout << "\n\nVetor Y (" << y.size() << " registros)\n";
    print_vector(y);

    std::cout << "\n\nVetor Z (" << z.size() << " registros)\n";
    print_vector(z);

    std::cout << "\n\nVetor Time (" << time.size() << " registros)\n";
    print_vector(time);

    std::cout << std::endl;
*/

    // Como a função de FFT utiliza números complexos (classe COMPLEX), esta rotina converte-os.
    std::cout << "\nConvertendo para numeros complexos.\n";

    std::vector<cd> cx,cy,cz,ct;
//    std::vector<cd> cx,cy,cz;

    for( unsigned long int i=0; i<x.size(); i++) {
        std::complex<long double> cax(time[i], x[i]);
        cx.push_back(cax);

        std::complex<long double> cay(time[i], y[i]);
        cy.push_back(cay);

        std::complex<long double> caz(time[i], z[i]);
        cz.push_back(caz);

        std::complex<long double> cat(time[i], total[i]);
        ct.push_back(cat);

    }

    std::cout << "Computando as FFT dos vetores X.\n";
    #define INVERT false
    fft2(cx,INVERT);
    fft2(cy,INVERT);
    fft2(cz,INVERT);
    fft2(ct,INVERT);


/*
    std::cout << "\nImprimindo:\n";
//    std::cout << std::endl;

    std::cout << "\nFFT(X)\n";
    for( cd x : cx) {
        std::cout << x << ";";
    }
    std::cout << std::endl;

    std::cout << "FFT(Y)\n";
    for( cd x : cy) {
        std::cout << x << ";";
    }
    std::cout << std::endl;

    std::cout << "FFT(Z)\n";
    for( cd x : cz) {
        std::cout << x << ";";
    }
    std::cout << std::endl;

    std::cout << "FFT(TOTAL)\n";
    for( cd x : ct) {
        std::cout << x << ";";
    }
*/

    std::cout << std::endl;
    std::cout << "FFT(X)[:3] = " << cx[0] << cx[1] << cx[2] << " ... " << std::endl;
    std::cout << "FFT(Y)[:3] = " << cy[0] << cy[1] << cy[2] << " ... " << std::endl;
    std::cout << "FFT(Z)[:3] = " << cz[0] << cz[1] << cz[2] << " ... " <<  std::endl;
    std::cout << "FFT(T)[:3] = " << ct[0] << ct[1] << ct[2] << " ... " <<  std::endl;
    std::cout << std::endl;

    // Grava arquivo output.txt
    std::cout << "Gravando dados calculados em output.txt";

    std::ofstream output;
    output.open("output.txt", std::ios::out);
//    output << "x,y,z,freq" << std::endl; // Cabeçalho do arquivo output.txt, comentar esta linha caso queira o output.txt sem cabeçalho
    for(long int i=0; i<x.size(); i++) {
	output << boost::format{"%1%,%2%,%3%,%4%"} % x[i] % y[i] % z[i] % fabs(ct[i].real()) << std::endl;
    }
    output.close();
    std::cout << "\n********************************************************************************************\n";

    return 0;
}
