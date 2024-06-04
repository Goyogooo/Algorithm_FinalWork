#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <smmintrin.h>  // SSE4.1 intrinsics
using namespace std;

// 读取小端格式的四字节无符号整数
uint32_t read_uint32_le(std::ifstream& stream) {
    uint32_t value;
    char bytes[4];
    stream.read(bytes, 4);
    value = (static_cast<uint32_t>(static_cast<unsigned char>(bytes[3])) << 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[2])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[1])) << 8) |
        static_cast<uint32_t>(static_cast<unsigned char>(bytes[0]));
    return value;
}

// 读取一个整数数组
std::vector<uint32_t> read_array(std::ifstream& stream) {
    uint32_t length = read_uint32_le(stream);
    std::vector<uint32_t> array(length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = read_uint32_le(stream);
    }
    return array;
}

int main() {
  //  for (int n = 0; n < 10; n++) {
        std::ifstream file("D:/MyVS/bingxingSIMDRBM/ExpIndex", std::ios::binary);
        if (!file) {
            std::cerr << "无法打开文件" << std::endl;
            return 1;
        }
        file.seekg(32832, std::ios::beg);
        std::vector<uint32_t> array = read_array(file);
        uint32_t length = array.size();

        map<uint32_t, vector<uint32_t>> compress;
        size_t thelength = length - (length % 4);
        __m128i* thearray = (__m128i*) array.data();

        for (size_t i = 0; i < thelength / 4; ++i) {
            __m128i chunk = _mm_load_si128(&thearray[i]);

            __m128i high_parts = _mm_srli_epi32(chunk, 16);

            __m128i low_mask = _mm_set1_epi32(0xFFFF);
            __m128i low_parts = _mm_and_si128(chunk, low_mask);



            for (int j = 0; j < 4; ++j) {
                uint32_t high_part;
                uint32_t low_part;
                switch (j) {
                case 0:
                    high_part = _mm_extract_epi32(high_parts, 0);
                    low_part = _mm_extract_epi32(low_parts, 0);
                    break;
                case 1:
                    high_part = _mm_extract_epi32(high_parts, 1);
                    low_part = _mm_extract_epi32(low_parts, 1);
                    break;
                case 2:
                    high_part = _mm_extract_epi32(high_parts, 2);
                    low_part = _mm_extract_epi32(low_parts, 2);
                    break;
                case 3:
                    high_part = _mm_extract_epi32(high_parts, 3);
                    low_part = _mm_extract_epi32(low_parts, 3);
                    break;
                }

                // 更新 map
                if (!compress.count(high_part)) {
                    compress[high_part] = vector<uint32_t>();
                }
                compress[high_part].push_back(low_part);
            }
        }

        for (uint32_t i = thelength; i < length; i++) {
            uint32_t HighPart = array[i] >> 16;
            uint32_t LowPart = array[i] & (65535);

            if (compress.find(HighPart) == compress.end()) {
                compress[HighPart] = vector<uint32_t>();
            }

            compress[HighPart].push_back(LowPart);
        }
        
        std::ofstream f("D:/MyVS/SFBX_RBM_SSE/compress.txt", std::ios::app);
        if (!f.is_open()) {
            std::cerr << "无法打开文件" << std::endl;
            return 0;
        }
        for (const auto& pair : compress) {
            f << "High Part: " << pair.first << " -> Low Parts: ";
            for (size_t i = 0; i < pair.second.size(); ++i) {
                f << pair.second[i];
                if (i != pair.second.size() - 1) f << ", ";
            }
            f << std::endl;
        }
        f.close();



       
        file.close();
   // }
    return 0;
}