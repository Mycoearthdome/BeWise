#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <vector>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <limits>

std::vector<std::string> split(std::string tokenized, int64_t size, std::string delimiter){
    size_t pos = 0;
    size_t previous_pos = 0;
    std::vector<std::string> tokens;
    int64_t j = 0;
    while ((pos = tokenized.find(delimiter, previous_pos)) != std::string::npos) {
        tokens.push_back(tokenized.substr(previous_pos, pos-previous_pos));
        j++;
        previous_pos = pos + 1;
    }
    return tokens;
}

std::string segments(std::string binary, std::string delimiter)
{
    std::string binary2;
    int64_t j = 0;
    int64_t k = 1;

    for (int64_t i=0;i<binary.length();i++){
        binary2 += binary[j];
        j++;
        if (k == 8){
            binary2 += delimiter;
            k = 0;
        }
        k++;
    }
    return binary2;
}

bool compare(std::string bin_header, std::string match){
    int j = 0;
    for (int i=0;i<256;i++){
        std::bitset<8> bits(j);
        if ((bin_header + bits.to_string()) == match){
            return true;
        }
        j++;
    }
    return false;
}

std::string pack(std::string binary)
{
    std::string delimiter = "|";
    int64_t size = binary.length()/8;
    std::string packed;
    std::string tokenized = segments(binary, delimiter);
    //std::string bit_posibilities[255];
    //std::cout << tokenized <<std::endl;
    std::vector<std::string> binary_list;
    binary_list = split(tokenized,size,delimiter);
    int64_t n4bits = binary_list.size();
    int64_t j = 0;
    for (int64_t i=0;i<n4bits;i++){
       //std::cout << binary_list[j] << std::endl;
       std::string remainingbits = std::string(binary_list[j],4,4);
       std::string comparebits  = std::string(binary_list[j], 0, 4);
       //std::cout << comparebits << "--->" << remainingbits << std::endl;
       //exit(0);
       if ("1001" == comparebits){        // 9
           binary_list[j] = "00000111" + remainingbits;    // becomes 7
       } else if ("1010" == comparebits){  //10
           binary_list[j] = "00000110" + remainingbits;    // becomes 6
       } else if ("1011" == comparebits){ //11
           binary_list[j] = "00001001" + remainingbits;    // becomes 9
       } else if ("1100" == comparebits){ //12
           binary_list[j] = "00000100" + remainingbits;    // becomes 4
       } else if ("1101" == comparebits){ //13
           binary_list[j] = "00001011" + remainingbits;    // becomes 11
       //} else if ("1110" == comparebits){ //14
       //    binary_list[j] = "00001010" + remainingbits;    // becomes 10
       } else if ("1111" == comparebits){ //15
           binary_list[j] = "00000101" + remainingbits;    // becomes 5
       } else if ("0000" == comparebits && std::stoi(remainingbits, nullptr, 2) != 10 && std::stoi(remainingbits, nullptr, 2) != 13){ //LF +CR
           binary_list[j] = "00000000" + remainingbits;                    // stays 0
       }
       packed += binary_list[j];
       j++;
    }
    return packed;
}

std::string readFile(FILE* stdin){
	std::string content;
	    std::vector<char> buffer(4096);

	    while (true) {
	        std::cin.read(buffer.data(), buffer.size());
	        std::streamsize bytesRead = std::cin.gcount();

	        if (bytesRead > 0) {
	            content.append(buffer.data(), static_cast<size_t>(bytesRead));
	        }

	        if (std::cin.eof()) {
	            break;
	        }
	    }
	return content;
}

std::string PrepData(FILE* stdin){
    std::string sData = readFile(stdin);
    std::string binary2 = "";

    int64_t j = 0;
    std::string binary;
    if (sData.length() < std::numeric_limits<int64_t>::max()){ //~9 Exabytes
    	for (int64_t i = 0;i<sData.length();i++){
    		unsigned char chr = sData[j];
    		std::bitset<8> bits(chr);
    		binary += bits.to_string();
    		j++;
    	}
    binary2 = pack(binary);
    } else {
    	std::cout<<"ERROR:Filesize limit is-->"<<std::numeric_limits<int64_t>::max()<<std::endl;;
    	exit(0);
    }

    return binary2;
}

std::vector<std::string> RecoverData(FILE* stdin){
    std::string sData = readFile(stdin);
    std::vector<std::string> Preppedbits;
    std::string bits;
    int64_t j = 0;
    int64_t seek = 0;
    if (sData.length() < std::numeric_limits<int64_t>::max()){ //~9 Exabytes
    for (int64_t i=0;i<sData.length();i++){
        bool adjustedValue = false;
        bits += sData[seek];
        seek++;
        if (j == 3 && seek+j < sData.length()){
            if (std::stoi(bits,nullptr, 2) == 0){
                // leading 4 zeros found
                for(int k=0;k<4;k++){
                    bits += sData[seek+k];
                }
                if (std::stoi(bits,nullptr, 2) == 0){
                    seek = seek + 4;
                    bits = "0000";
                    for (int k=0;k<4;k++){
                        bits += sData[seek+k];
                    }
                    adjustedValue = true;
                }
                int value = std::stoi(bits, nullptr,2);
                if ((value >= 4 && value != 8 && value != 10 && value <= 11) && !adjustedValue){
                    //then it's probably a 12 digit I forged.
                    std::string forgedbits = std::string(bits,4,4);
                    std::bitset<4> forged(forgedbits);
                    forged = (forged <<1)^forged; //the magic (GOOD)
                    bits = forged.to_string();
                    seek += 4;
                    for (int l=0;l<4;l++){
                        bits += sData[seek+l];
                    }
                    seek += 4;
                } else {
                    seek += 4;
                }
                Preppedbits.push_back(bits);
                bits = "";
            } else {
                for(int k=0;k<4;k++){
                    bits += sData[seek+k]; //seek
                }
                Preppedbits.push_back(bits);
                seek += 4;
            }
            j = -1;
            bits = "";
        } else if (seek+j >= sData.length()){
        	break;
        }
        j++;
    }
    } else {
    	std::cout<<"ERROR:Filesize limit is-->"<<std::numeric_limits<int64_t>::max()<<std::endl;;
    	exit(0);
    }
    return Preppedbits;
}



int main(int argc, char **argv)
{
	std::string file;
	FILE* stdin;
	std::bitset<8> bits;
	try{
	if (argc==1){
		//output the bits content
		std::cout << PrepData(stdin);
	} else if (argc > 1){
		std::vector<std::string> Preppedbits = RecoverData(stdin);
		//recover the byte contents
		for (std::string bitstring : Preppedbits){
				bits = std::bitset<8>(bitstring);
		        unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
		        std::cout << byte;
		    }
	}
	}	catch (const std::runtime_error& e) {
		        std::cerr << "Exception caught: " << e.what() << std::endl;

	}


    return 0;
}
