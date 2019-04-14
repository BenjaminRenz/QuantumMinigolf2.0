#include "filereader.h"
#include <stdlib.h>

unsigned int read_uint_from_endian_file(FILE* file) {
    unsigned char data[4];
    unsigned int data_return_int;
    if(fread(data, 1, 4, file) < 4) {     //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_int = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_int=(data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
    return data_return_int;
}

unsigned short read_short_from_endian_file(FILE* file) {
    //http://cpansearch.perl.org/src/DHUNT/PDL-Planet-0.05/libimage/bmp.c
    unsigned char data[2];
    unsigned short data_return_short;
    if(fread(data, 1, 2, file) < 2) {     //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_short = (data[1] << 8) | data[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_short=(data[0]<<8)|data[1];
    return data_return_short;
}

unsigned char* read_bmp(char* filepath) {
    //source https://github.com/ndd314/sjsu_cmpe295_cuda_fft_opengl/blob/master/opengl/plane/readBMPV2.c#L50
    //https://stackoverflow.com/questions/7990273/converting-256-color-bitmap-to-rgba-bitmap
    FILE *filepointer = fopen(filepath, "rb");

    if(filepointer == NULL) {
        printf("Error: File :%s could not be found\n", filepath);
        fclose(filepointer);
        return 0;
    }

    fseek(filepointer, 0, SEEK_SET);   //Jump to beginning of file
    if(read_short_from_endian_file(filepointer) != 0x4D42) {     // (equals BM in ASCII)
        fclose(filepointer);
        printf("Error: File :%s is not an BMP\n", filepath);
        return 0;
    }

    fseek(filepointer, 10, SEEK_SET);
    unsigned int BitmapOffset = read_uint_from_endian_file(filepointer);
    printf("Info: BMP data offset:%d\n", BitmapOffset);

    if(read_uint_from_endian_file(filepointer) != 124) {
        printf("Error: BitmapHeader is not BITMAPV5HEADER / 124 \n");
        fclose(filepointer);
        return 0;
    }
    unsigned int BitmapWidth = read_uint_from_endian_file(filepointer);
    printf("Info: BitmapWidth is %d.\n", BitmapWidth);

    unsigned int BitmapHeight = read_uint_from_endian_file(filepointer);
    printf("Info: BitmapHeight is %d.\n", BitmapHeight);

    if(read_short_from_endian_file(filepointer) != 1) {
        printf("Error: Unsupported plane count\n");
        return 0;
    }

    unsigned int BitmapColorDepth = read_short_from_endian_file(filepointer);
    printf("Info: BMP color depth:%d", BitmapColorDepth);
    unsigned int BitmapSizeCalculated = (BitmapColorDepth / 8) * (BitmapWidth + (BitmapWidth % 4)) * BitmapWidth;

    unsigned int BitmapCompression = read_uint_from_endian_file(filepointer);
    switch(BitmapCompression) {
    case 0:
        printf("Error: Compression type: none/BI_RGB\n");
        fclose(filepointer);
        return 0; //TODO add support
        break;
    case 3:
        printf("Info: Compression type: Bitfields/BI_BITFIELDS\n");
        break;
    default:
        printf("Error: Unsupported compression %d\n", BitmapCompression);
        fclose(filepointer);
        return 0;
        break;
    }
    unsigned int BitmapImageSize = read_uint_from_endian_file(filepointer);
    if(BitmapImageSize != BitmapSizeCalculated) {
        printf("Error: Reading image size: Calculated Image Size: %d.\nRead Image size: %d\n", BitmapSizeCalculated, BitmapImageSize);
        fclose(filepointer);
        return 0;
    }
    printf("Info: Image Size:%d\n", BitmapSizeCalculated);
    /*unsigned int BitmapXPpM=*/read_uint_from_endian_file(filepointer);
    /*unsigned int BitmapYPpM=*/read_uint_from_endian_file(filepointer);
    unsigned int BitmapColorsInPalette = read_uint_from_endian_file(filepointer);
    printf("Info: Colors in palette: %d.\n", BitmapColorsInPalette);
    fseek(filepointer, 4, SEEK_CUR);   //skip over important color count
    if(BitmapCompression == 3) {
        unsigned char RGBA_mask[4];
        for(unsigned int color_channel = 0; color_channel < 4; color_channel++) {
            unsigned int color_channel_mask = read_uint_from_endian_file(filepointer);
            switch(color_channel_mask) {   //read shift value for color_channel
            case 0xFF000000:
                RGBA_mask[color_channel] = 3; //transparency
                break;
            case 0x00FF0000:
                RGBA_mask[color_channel] = 2; //b
                break;
            case 0x0000FF00:
                RGBA_mask[color_channel] = 1; //g
                break;
            case 0x000000FF:
                RGBA_mask[color_channel] = 0; //r
                break;
            default:
                printf("Error: Invalid BITMASK. Value: %x!\n", color_channel_mask);
                fclose(filepointer);
                return 0;
                break;
            }
        }
        printf("Info: Shifting value for R:%d G:%d B:%d A:%d\n", RGBA_mask[0], RGBA_mask[1], RGBA_mask[2], RGBA_mask[3]);
        uint8_t* imageData = malloc(BitmapSizeCalculated);
        printf("Info: BMOFFST: %d\n", BitmapOffset);
        fseek(filepointer, BitmapOffset, SEEK_SET);   //jump to pixel data
        printf("Info: Calsize: %d\n", BitmapSizeCalculated);
        if(fread(imageData, BitmapSizeCalculated, 1, filepointer) == 0) {
            printf("Error: Reading failed!");
        }
        int colorOrder[4]={3,2,1,0};
        for(int i=0;i<(BitmapSizeCalculated/4);i++){
            uint32_t imageDataTemp=0;
            for(int colorchannel=0;colorchannel<4;colorchannel++){
                int shiftlength_old=(RGBA_mask[colorchannel]*8);
                int shiftlenght_new=(colorOrder[colorchannel]*8);
                imageDataTemp+=((((uint32_t*)imageData)[i]&(0xFF<<shiftlength_old))>>shiftlength_old)<<shiftlenght_new;
            }
            ((uint32_t*)imageData)[i]=imageDataTemp;
        }
        fclose(filepointer);
        return imageData;
    }
    printf("Error: Currently not implemented!");
    fclose(filepointer);
    return 0;
}

//NOT READY YET
void write_bmp(char* filepath, unsigned int width, unsigned int height) {
    FILE* filepointer = fopen(filepath, "wb");
    //bytes_per_line=(3*(width+1)/4)*4;
    const char* String_to_write = "BMP";
    fwrite(&String_to_write, sizeof(char), 3, filepointer);
    return;
}
