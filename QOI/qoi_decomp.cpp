#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

uint32_t le2be(uint32_t n_l, size_t nbytes) {
    uint32_t n = 0;
    n = n + (n_l & 0b11111111);
    if (nbytes > 0) {
        uint8_t shifter = 8;
        for (size_t i = 0; i < nbytes - 1; ++i) {
            n = (n << 8) + ((n_l >> shifter) & 0b11111111);
            shifter += 8;
        }
        return n;
    }
    std::cout << "Number of bytes <= 0 during little endian to big endian is not acceptable." << std::endl;
    return EXIT_FAILURE;
}

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows* cols) {}

    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

int main(int argc, char* argv[])
{
    std::ifstream is(argv[1], std::ios::binary);
    if (!is) {
        std::cout << "Error: cannot open input file" << std::endl;
    }


    //read header and image size

    char magic[4];
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t colorspace;
    is.read(reinterpret_cast<char*>(&magic[0]), 4);
    is.read(reinterpret_cast<char*>(&width), 4);
    is.read(reinterpret_cast<char*>(&height), 4);
    is.read(reinterpret_cast<char*>(&channels), 1);
    is.read(reinterpret_cast<char*>(&width), 1);
    width = le2be(width, 4);
    height = le2be(height, 4);

    using rgba = std::array<uint8_t, 4>;
    mat<rgba> img(height, width); 

    // decodificare il file QOI in input e inserire i dati nell'immagine di output
    
    size_t n = 0;
    std::vector<rgba> array(64);
    uint8_t init_r, init_g, init_b, init_a;
    init_r = 0;
    init_g = 0;
    init_b = 0;
    init_a = 255;


    while (1) {
        std::vector<uint8_t> chunk(5);
        is.read(reinterpret_cast<char*>(chunk.data()), 1);
        
        if (chunk[0] == 0x00) {
            if (is.peek() == 0x00) {
                break;
            }
        }

        //QOI_OP_RGBA
        if (chunk[0] == 255) {
            is.read(reinterpret_cast<char*>(&chunk[1]), 4);
            uint8_t r, g, b, a;
            r = chunk[1];
            g = chunk[2];
            b = chunk[3];
            a = chunk[4];

            //save pixel inside the array-64
            uint8_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            array[index_position] = rgba{ r, g, b, a };

            //write pixel on new image
            img[n] = rgba{ r, g, b, a };
            ++n;
        }

        //QOI_OP_RGB
        else if (chunk[0] == uint8_t(254)) {
            is.read(reinterpret_cast<char*>(&chunk[1]), 3);
            uint8_t r, g, b;
            r = chunk[1];
            g = chunk[2];
            b = chunk[3];

            //save pixel inside the array-64

            if (n == 0) {
                uint8_t index_position = (r * 3 + g * 5 + b * 7 + init_a * 11) % 64;
                array[index_position] = rgba{ r, g, b, init_a };
            }
            else {
                uint8_t index_position = (r * 3 + g * 5 + b * 7 + img[n-1][3] * 11) % 64;
                array[index_position] = rgba{ r, g, b, img[n-1][3]};
            }
            
            //write pixel on new image
            img[n] = rgba{ r, g, b, init_a };
            ++n;
        }
        else {
            uint8_t tag = 0;
            tag = (chunk[0] & 0b11000000) >> 6;

            //QOI_OP_INDEX
            if (tag == 0) {
                uint8_t index = chunk[0] & 0b00111111;
                img[n] = array[index];
                ++n;
            }

            //QOI_OP_DIFF
            else if (tag == 1) {
                std::vector<int8_t> v(3);
                v[0] = ((chunk[0] & 0b00110000) >> 4) - 2;
                v[1] = ((chunk[0] & 0b00001100) >> 2) - 2;
                v[2] = (chunk[0] & 0b00000011) - 2;

                rgba prev;
                if (n == 0) {
                    prev = { init_r, init_g, init_b, init_a };
                }else{
                    prev = img[n - 1];
                }

                for (size_t j = 0; j < 3; ++j) {
                    if (prev[j] + v[j] > 255) {
                        prev[j] = v[j] - 1 - (255 - prev[j]);
                    }
                    else if (prev[j] + v[j] < 0) {
                        prev[j] = 255 - (v[j] + prev[j] - 1);
                    }
                    else {
                        prev[j] += v[j];
                    }
                }
                img[n] = prev;
                uint8_t index_position = (prev[0] * 3 + prev[1] * 5 + prev[2] * 7 + prev[3] * 11) % 64;
                array[index_position] = prev;
                ++n;
            }
            //QOI_OP_LUMA
            else if (tag == 2) {
                is.read(reinterpret_cast<char*>(&chunk[1]), 1);
                int8_t dg = 0;
                int8_t dr_dg = 0;
                int8_t db_dg = 0;
                dg = (chunk[0] & 0b00111111) - 32;
                dr_dg = ((chunk[1] & 0b11110000) >> 4) - 8;
                db_dg = (chunk[1] & 0b00001111) - 8;

                rgba prev;
                if (n == 0) {
                    prev = { init_r, init_g, init_b, init_a };
                }
                else {
                    prev = img[n - 1];
                }

                //write new green value
                if (prev[1] + dg > 255) {
                    prev[1] = dg - 1 - (255 - prev[1]);
                }
                else if (prev[1] + dg < 0) {
                    prev[1] = 255 - (dg + prev[1] - 1);
                }
                else {
                    prev[1] += dg;
                }

                //write red 
                if (prev[0] + dr_dg + dg > 255) {
                    prev[0] = dg - dr_dg - 1 - (255 - prev[0]);
                }
                else if (prev[0] + dr_dg + dg < 0) {
                    prev[0] = 255 - (dg + dr_dg + prev[0] - 1);
                }
                else {
                    prev[0] = prev[0] + dr_dg + dg;
                }

                //write blue
                if (prev[2] + db_dg + dg > 255) {
                    prev[2] = dg - db_dg - 1 - (255 - prev[2]);
                }
                else if (prev[2] + db_dg + dg < 0) {
                    prev[2] = 255 - (dg + db_dg + prev[2] - 1);
                }
                else {
                    prev[2] = prev[2] + db_dg + dg;
                }

                img[n] = prev;
                uint8_t index_position = (prev[0] * 3 + prev[1] * 5 + prev[2] * 7 + prev[3] * 11) % 64;
                array[index_position] = prev;
                ++n;
            }
            //QOI_OP_RUN
            else {
                rgba prev;
                if (n == 0) {
                    prev = { init_r, init_g, init_b, init_a };
                }
                else {
                    prev = img[n - 1];
                }

                uint8_t run = 0;
                run = (chunk[0] & 0b00111111) + 1;
                for (size_t j = 0; j < run; ++j) {
                    img[n] = prev;
                    ++n;
                }
            }   
        }
    }


    // Questo è il formato di output PAM. È già pronto così, ma potete modificarlo se volete
    std::ofstream os(argv[2], std::ios::binary); // Questo utilizza il secondo parametro della linea di comando!
    os << "P7\nWIDTH " << +img.cols() << "\nHEIGHT " << +img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}