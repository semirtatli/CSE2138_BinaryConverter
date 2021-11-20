/*
 * Abdül Samet Yýlmaz 150118059
 * Semir TatlÄ± 150119004
 * Muhammet Zübeyir Ýncekara 150119038
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define INTEGER_SIZE 16 // Integers will be stored in 16 bits
#define BINARY_SIZE 32 // All numbers will be stored in 32 bits
#define WORD_SIZE 32 // A word can contain maximum 32 characters
#define LINE_SIZE 512 // A line can contain maximum 512 characters
#define NUMBER_COUNT 64 // An input file can contain maximum 64 numbers

typedef struct Number {
    double number;
    unsigned short isSigned;
    unsigned short isFloatingPoint;
    unsigned short isNotNull;
    unsigned short binary[BINARY_SIZE];
    char hex[BINARY_SIZE / 4];
} Number;

void readFileName();

void readFile(char filename[]);

void readByteOrdering();

void readFloatingPointSize();

void setNumber(Number *number, char line[LINE_SIZE]);

void printArray();

void printHex(Number number);

void strToLower(char string[]);

void unsignedToBinary(double number, unsigned short *binary, unsigned short numOfBits);

void signedToBinary(double number, unsigned short *binary, unsigned short numOfBits);

void floatToBinary(double number, unsigned short *binary);

void binaryToHex(const unsigned short *binary, char *hex, unsigned short numOfBits);

void saveOutput();

char inputFile[WORD_SIZE] = "input.txt";
char outputFile[WORD_SIZE] = "output.txt";
unsigned short isLittleEndian = 1;
unsigned short floatingPointSize = 2;
unsigned short floatingPointBits = 16;

Number numbers[NUMBER_COUNT];

int main() {
    readFileName();
    readByteOrdering();
    readFloatingPointSize();
    readFile(inputFile);
    printArray(numbers);
    saveOutput();
    return 0;
}

void readFileName() {
    printf("Enter the input file name:\n");
    scanf("%s", inputFile);
}

void readFile(char filename[]) {
    char line[LINE_SIZE];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    unsigned short i = 0;
    while (fscanf(fp, "%s", line) != EOF) {
        setNumber(&numbers[i], line);
        i++;
    }
    fclose(fp);
}

void setNumber(Number *number, char line[LINE_SIZE]) {
    sscanf(line, "%lf", &number->number);
    number->isSigned = strchr(line, 'u') != NULL ? 0 : 1;
    number->isFloatingPoint = strchr(line, '.') != NULL ? 1 : 0;
    number->isNotNull = 1;
    if (!number->isFloatingPoint) {
        if (number->isSigned) {
            signedToBinary(number->number, number->binary, INTEGER_SIZE);
        } else {
            unsignedToBinary(number->number, number->binary, INTEGER_SIZE);
        }
    } else {
        floatToBinary(number->number, number->binary);
    }
    int limit = (number->isFloatingPoint) ? floatingPointBits : INTEGER_SIZE;
    binaryToHex(number->binary, number->hex, limit);
}

void strToLower(char string[]) {
    unsigned short i;
    for (i = 0; string[i] != '\0'; i++) {
        string[i] = tolower(string[i]);
    }
}

void readByteOrdering() {
    printf("Enter the byte ordering (l/b):\n");
    char input[WORD_SIZE];
    scanf("%s", input);
    strToLower(input);
    if (strcmp("l", input) == 0) {
        isLittleEndian = 1;
    } else if (strcmp("b", input) == 0) {
        isLittleEndian = 0;
    } else {
        printf("Only two options: little or big !!\n");
        readByteOrdering();
    }
}

void readFloatingPointSize() {
    printf("Enter the floating point size (1-4):\n");
    scanf("%hu", &floatingPointSize);
    if (floatingPointSize <= 0 || floatingPointSize > 4) {
        printf("You can only enter number from 1 to 4\n");
        readFloatingPointSize();
    }
    floatingPointBits = floatingPointSize * 8;
}

void unsignedToBinary(double number, unsigned short *binary, unsigned short numOfBits) {
    int unsign = (int) (number);
    int i;
    int j;
    for (i = numOfBits - 1, j = 0; i >= 0; i--, j++) {
        int bit = unsign >> i;
        binary[j] = bit & 1 ? 1 : 0;
    }
}

void signedToBinary(double number, unsigned short *binary, unsigned short numOfBits) {
    if (number >= 0) {
        unsignedToBinary(number, binary, numOfBits);
    } else {
        // If number is negative convert it to positive and find binary of it
        number *= -1;
        unsignedToBinary(number, binary, numOfBits);
        // Flip bits
        unsigned short i;
        for (i = 0; i < numOfBits; i++) {
            binary[i] = (binary[i] == 0) ? 1 : 0;
        }
        // Add 1
        for (i = numOfBits - 1; i > 0; i--) {
            binary[i] = (binary[i] == 0) ? 1 : 0;
            if (binary[i]) {
                break;
            }
        }
    }
}

double getFractionPart(double number) {
    if (number < 0) {
        number *= -1;
    }
    return number - (int) number;
}

unsigned short numberOfExponentBits() {
    unsigned short exponentBits;
    switch (floatingPointSize) {
        case 1:
            exponentBits = 4;
            break;
        case 2:
        default:
            exponentBits = 6;
            break;
        case 3:
            exponentBits = 8;
            break;
        case 4:
            exponentBits = 10;
            break;
    }
    return exponentBits;
}

int roundFraction(unsigned int bitsNeeded, unsigned short *fraction, int num, unsigned short fractionBits) {
    int expanded = 0;
    unsigned int i;
    unsigned short halfWay = 1;
    if (fraction[num + 1] == 0) {
        return 0;

    } else {
        for (i = num + 2; i < bitsNeeded; i++) {
            if (fraction[i] == 1) {
                halfWay = 0;
            }
        }
        if (halfWay == 0 && fractionBits >= 0) {
            for (i = fractionBits -1; i > 0; i--) {
                if (fraction[i] == 0) {
                    fraction[i] = 1;
                    return 0;
                } else {
                    fraction[i] = 0;
                    if (i == 1) {
                        fraction[i] = 1;
                        expanded = 1;
                    }
                }
            }
        }
        if (halfWay == 1 && fractionBits >= 0) {
            if (fraction[num] == 0)
                return 0;
            else {
                for (i = fractionBits; i > 0; i--) {
                    if (fraction[i] == 0) {
                        fraction[i] = 1;
                        return 0;
                    } else {
                        fraction[i] = 0;
                        if (i == 1) {
                            fraction[i] = 1;
                            expanded = 1;
                        }
                    }
                }
            }
        }
    }
    if (expanded == 1)
        return 1;
    else
        return 0;
}

int getIntegerPart(double number) {
    return (int) number;
}

void fractionToBinary(double number, unsigned short *binary, unsigned short numOfBits) {
    double fractionPart = getFractionPart(number);
    int i, j;
    for (i = numOfBits - 1, j = 0; i >= 0; i--, j++) {
        double multiplyFraction = fractionPart * 2.0;
        if (multiplyFraction >= 1.0) {
            binary[j] = 1;
        } else {
            binary[j] = 0;
        }
        fractionPart = getFractionPart(multiplyFraction);
    }
}

short findE(const unsigned short *b1, const unsigned short *b2, unsigned short s1, unsigned short s2) {
    short E = 0;
    unsigned short i = 0;
    unsigned short j;
    // Search for 1 in binary of integer part
    for (j = 0; j < s1; j++, i++) {
        if (b1[j] == 1) {
            E = (short) i;
            // Found first 1 in ith position, convert it to IEEE format (2^E)
            E = (short) (s1 - (E + 1));
            return E;
        }
    }
    // If 1 is not found in integer part check fraction binary
    i = -1;
    for (j = 0; j < s2; j++, i--) {
        if (b2[j] == 1) {
            E = (short) i;
            return E;
        }
    }
    // If not found both integer and fraction part E equals to zero
    return E;
}

unsigned short calculateBias(unsigned short numOfExponentBits) {
    return (unsigned short) (pow(2, numOfExponentBits - 1) - 1);
}

void
createMantissa(unsigned short *mantissa, short E, unsigned short fractionBits, const unsigned short *b1,
               const unsigned short *b2,
               unsigned short numOfIntBits) {
    // Eg. 1011.0011 b1: 1011 (integer part) b2:0011 (fraction part)
    unsigned short i = 0;
    unsigned short j;
    // If E less than zero this means we do not have bits in binary of integer part to use in mantissa
    // Eg. 0.01111 => 1.111*2^-2
    if (E < 0) {
        for (j = (E * -1); j < fractionBits; j++, i++) {
            mantissa[i] = b2[j];
        }
    } else {
        // If E is positive we will start from binary of integer part
        for (j = numOfIntBits - E; j < numOfIntBits; j++, i++) {
            mantissa[i] = b1[j];
        }
        for (j = 0; j < fractionBits; j++, i++) {
            mantissa[i] = b2[j];
        }
    }
    // If empty bits left fill them with zero
    while (i < fractionBits) {
        mantissa[i] = 0;
        i++;
    }
}

void floatToBinary(double number, unsigned short *binary) {
    unsigned short signBit = 0;
    if (number < 0) {
        signBit = 1;
        number *= -1;
    }
    // Get integer part of number
    double integerPart = getIntegerPart(number);
    // Find how many bits needed for represent integer part in base 2
    double base2 = log(number) / log(2.0);
    unsigned short integerBinaryBits = (integerPart < 1) ? 1 : (int) ceil(base2);
    if (getFractionPart(base2) == 0) {
        integerBinaryBits++;
    }
    // Array for storing bits of integer part
    unsigned short integerBinary[integerBinaryBits];
    unsignedToBinary(integerPart, integerBinary, integerBinaryBits);
    // Get how many exponent bits will be used in floating point
    unsigned short numOfExponentBits = numberOfExponentBits();
    // Calculate how many fraction bits will be available
    unsigned short fractionBits;
    if (floatingPointSize == 1 || floatingPointSize == 2){
        fractionBits = floatingPointBits - numOfExponentBits;
    }
    else{
        fractionBits = 14;
    }

    // Create an array for fraction part, but add 3 more bits for accuracy. After we can round it.
    unsigned short fractionBinary[fractionBits + 3];
    // Convert fraction part of number to binary
    fractionToBinary(number, fractionBinary, fractionBits + 3);
    unsigned short mantissa[fractionBits];
    short E = findE(integerBinary, fractionBinary, integerBinaryBits, fractionBits);
    // Fill fraction bits
    createMantissa(mantissa, E, fractionBits, integerBinary, fractionBinary, integerBinaryBits);
    roundFraction(fractionBits + 2 + integerBinaryBits, mantissa, fractionBits - integerBinaryBits + 2, fractionBits);
    // Find how many times point will be floated. Examples:
    // 111.0001 => 1.110001 E=2 | 0.01001 => 1.001 E=-2
    // If E less than zero, we will start mantissa part from fractionBinary[-E]
    // Calculate bias depends on number of exponent bits
    unsigned short bias = calculateBias(numOfExponentBits);
    // Calculate exponent part in base 10. Then we will convert it to binary.
    short e = bias + E;
    if (e == 0) {
        E = 1 - bias;
        createMantissa(mantissa, E, fractionBits, integerBinary, fractionBinary, integerBinaryBits);
        roundFraction(fractionBits + 2 + integerBinaryBits, mantissa, fractionBits - integerBinaryBits, fractionBits);
    }
    // Create an array for exponent bits then convert exponent to base 2.
    unsigned short exponentBinary[numOfExponentBits];
    unsignedToBinary(e, exponentBinary, numOfExponentBits);
    // First bit will be sign bit
    binary[0] = signBit;
    unsigned short i = 1;
    unsigned short j;
    unsigned short k;
    // Fill exponent bits
    for (j = 0; j < numOfExponentBits; j++, i++) {
        binary[i] = exponentBinary[j];
    }
    for (k = 0; k < fractionBits; k++, i++) {
        binary[i] = mantissa[k];
    }
}

void binaryToHex(const unsigned short *binary, char *hex, unsigned short numOfBits) {
    int hexSize = (int) ceil(numOfBits / 4.0);
    int hexValue = 0;
    int i, j, k;
    for (i = numOfBits - 1, j = 0, k = hexSize - 1; i > -1; i--, j++) {
        // Divide bits in groups of four and calculate value of them in base 10
        hexValue += (int) pow(2, j) * binary[i];
        if (j == 3) {
            // Find hex char
            hex[k] = (hexValue < 10) ? ('0' + hexValue) : ('A' + hexValue - 10);
            // Reset indexes
            j = -1;
            hexValue = 0;
            k--;
        }
    }
    // If need little endian reverse big endian
    if (isLittleEndian) {
        int i;
        char tempHex[hexSize];
        for (i = 0; i < hexSize; i++) {
            tempHex[i] = hex[i];
        }
        for (i = 0; i < hexSize; i += 2) {
            hex[i] = tempHex[hexSize - 2 - i];
            hex[i + 1] = tempHex[hexSize - 1 - i];
        }
    }
}

void printBinary(Number number) {
    int limit = (number.isFloatingPoint) ? floatingPointBits : INTEGER_SIZE;
    int i;
    for (i = 0; i < limit; i++) {
        printf("%d", number.binary[i]);
    }
}

void printArray() {
    unsigned short i;
    for (i = 0; i < NUMBER_COUNT && numbers[i].isNotNull == 1; i++) {
        printHex(numbers[i]);
        printf("\n");
    }
}

void printHex(Number number) {
    int limit = (number.isFloatingPoint) ? floatingPointBits : INTEGER_SIZE;
    limit = (int) ceil(limit / 4.0);
    int i, j;
    for (i = 0, j = 0; i < limit; i++, j++) {
        if (j == 2) {
            printf(" ");
            j = 0;
        }
        printf("%c", number.hex[i]);
    }
}

void saveOutput() {
    FILE *fp = fopen(outputFile, "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    unsigned short i;
    int k, j;
    for (i = 0; i < NUMBER_COUNT && numbers[i].isNotNull == 1; i++) {
        int limit = (numbers[i].isFloatingPoint) ? floatingPointBits : INTEGER_SIZE;
        limit = (int) ceil(limit / 4.0);
        for (k = 0, j = 0; k < limit; k++, j++) {
            if (j == 2) {
                fprintf(fp, " ");
                printf(" ");
                j = 0;
            }
            fprintf(fp, "%c", numbers[i].hex[k]);
        }
        if (numbers[i + 1].isNotNull == 1) {
            fprintf(fp, "\n");
        }
    }
    fclose(fp);
}
