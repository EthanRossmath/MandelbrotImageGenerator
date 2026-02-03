#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>

#pragma pack(push, 1)
// uint(#num)_t indicates that it is an unsigned integer of #num BITS long.
// remember, 1 byte = 8 bits.

struct  BMPFileHeader //must be 14 bytes long!
{
    uint16_t fileType{0x4D42};  // File type (always 0x424D) (2 bytes long)
                                // LITTLE ENDIAN (so backward 4D42)
    uint32_t fileSize{0};       // File size (little endian) (4 bytes long)
    uint16_t reserve1{0};       // Reserve1 (2 bytes) always 0
    uint16_t reserve2{0};       // Reserve2 (2 bytes) always 0
    uint32_t offSet{0};         // Offset of pixel data (4 bytes)
};

struct BMPInfoHeader // usually 40 bytes long
{
    uint32_t sizeInfoHeader{0};     // Size of bit map info header (4 bytes)
    int32_t imageWidth{0};          // Image width (4 bytes) (number of PIXELS per scanline)
    int32_t imageHeight{0};         // Image height (4 bytes) (number of scanlines)

    uint16_t biPlane{1};            // biplane (2 bytes) always 1
    uint16_t bitsPerPixel{24};      // bits per pixel (2 bytes) 24 is normal
    uint32_t imageCompression{0};   // compression (4 bytes) 0 means none
    uint32_t pixelDataSize{0};      // byte size of pixel section (4 bytes)
    int32_t horPixelPerMetre{0};    // Number of pixels per metre in the horizontal 
                                    // direction
    int32_t verPixelPerMetre{0};    // Number of pixels per metre in the vertical 
                                    // direction
    uint32_t coloursUsed{0};        // colours in colour index (4 bytes)
    uint32_t importantColours{0};   // important colours (4 bytes) set to 0
};

#pragma pack(pop)

//static asserts
static_assert(sizeof(BMPFileHeader) == 14, "BMPFileHeader must be exactly 14 bytes");

//create a pixel struct to abstract the logic of assigning colour to each pixel
struct Pixel
{
    uint8_t blue{};
    uint8_t green{};
    uint8_t red{};
};

// Defining the class BMP that will allow us to read/write .bmp files
class BMP
{
private:
    // Member variables
    BMPFileHeader m_fileHeader{};
    BMPInfoHeader m_infoHeader{};
    std::vector<Pixel> m_imageData{}; // will not store padding information

    //Helper functions

    //1. Compute the number of bytes in each scanline row dedicated to pixels
    int32_t pixelBytesPerRow()
    {
        return m_infoHeader.imageWidth * sizeof(Pixel);
    }

    //2. Computes the extra bytes needed per scanline to make sure each row is
    // a multiple of 4 bytes
    int32_t padding()
    {
        return (4 - (this->pixelBytesPerRow() % 4)) % 4;
        // The extra % 4 is there to handle the case when the pixel bytes in 
        // each row is a multiple of 4
    }

    //3. Extracts pixel information from the body of a .bmp file
    void readPixelData(std::ifstream& inf)
    {
        // The height is allowed to be negative in a .bmp file, meaning we
        // need to take the absolute value to make sure we don't get sign errors
        int32_t absHeight = std::abs(m_infoHeader.imageHeight);

        //Correctly size the imageData vector to hold only pixels (no padding)
        m_imageData.resize(absHeight * m_infoHeader.imageWidth);

        //Go line by line, reading the pixel data and discarding the padding
        for (int i{0}; i < absHeight; ++i)
        {
            // Depending on the sign of the image height, we will either fill in the rows
            // from top to bottom or bottom to top
            int row = (m_infoHeader.imageHeight > 0) ? i : (absHeight - i - 1);

            // Need to reinterpret each row of the image data vector as a pointer
            // to use the read function
            char* row_pointer = reinterpret_cast<char*>(&m_imageData[row * m_infoHeader.imageWidth]);

            // Reading the pixels into the row pointer
            inf.read(row_pointer, this->pixelBytesPerRow());

            // Ignore the padding
            inf.ignore(this->padding());
        }
    }

    //3. Writing pixel data to a .bmp file
    void writePixelData(std::ofstream& outf)
    {
        // Need to create a padding vector consisting of #padding number of 0 
        // bytes (could be anything)
        std::vector<uint8_t> paddingData(this->padding(), 0);

        //Taking the absolute value of height in case it's negative
        int32_t absHeight = std::abs(m_infoHeader.imageHeight);

        for (int i{0}; i < absHeight; ++i)
        {
            // Fill in rows top to bottom or bottom to top depending on the sign 
            // of the height
            int row = (m_infoHeader.imageHeight > 0) ? i : (absHeight - i - 1);

            // The write method requires character pointers as buffers to write into
            char* row_pointer = reinterpret_cast<char*>(&m_imageData[row 
            * m_infoHeader.imageWidth]);
            char* padding_pointer = reinterpret_cast<char*>(paddingData.data());

            // Writing the contents of row_pointer and padding_pointer into the file
            outf.write(row_pointer, this->pixelBytesPerRow());
            outf.write(padding_pointer, this->padding());
        }
    }


public:
    BMP(const char* fname)
    {
        read(fname);
    }

    void read(const char* fname)
    {
        std::ifstream inf{ fname, std::ios_base::binary };

        if (!inf)
        {
            std::cerr << "Could not read file '" << fname << "'\n";
            return ;
        }

        // Read file header into fileHeader.
        inf.read((char*)&m_fileHeader, sizeof(BMPFileHeader)); 
        assert(m_fileHeader.fileType == 0x4D42);

        // Read the info header as well.
        inf.read((char*)&m_infoHeader, sizeof(BMPInfoHeader));


        // Jumps to start of pixel data
        inf.seekg(m_fileHeader.offSet, inf.beg); // jumps to start of pixel data

        // Read data from inf into m_pixelData vector
        readPixelData(inf);
    }

    BMP(int32_t width, int32_t height)
    {
        // Set width and height of image (negative height means we work downwards)
        m_infoHeader.imageWidth = width;
        m_infoHeader.imageHeight = -height;

        // Read off info header size
        m_infoHeader.sizeInfoHeader = sizeof(BMPInfoHeader);

        // The offest begins after the two headers have ended
        m_fileHeader.offSet = m_infoHeader.sizeInfoHeader + sizeof(BMPFileHeader);
        
        // Compute total number of images bytes (including padding)
        m_infoHeader.pixelDataSize = height * (this->pixelBytesPerRow() + this->padding());

        // Resize the pixelData vector to hold exactly the correct number of bytes
        m_imageData.resize(height * width);

        // Combine all header and pixel data sizes
        m_fileHeader.fileSize = m_fileHeader.offSet + m_infoHeader.pixelDataSize;
    }

    void write(const std::string& fname)
    {
        std::ofstream outf{fname, std::ios_base::binary};

        if (!outf)
        {
            std::cerr << "Could not write to file '" << fname << "'\n";
        }

        //Write headers.
        outf.write((const char*)&m_fileHeader, sizeof(m_fileHeader));
        
        assert(m_fileHeader.fileType == 0x4D42);


        outf.write((const char*)&m_infoHeader, sizeof(m_infoHeader));

        //Write data
        writePixelData(outf);
    }

    Pixel& getPixel(int horPos, int VerPos)
    {
        return m_imageData[VerPos * m_infoHeader.imageWidth + horPos];
    }
    
};
