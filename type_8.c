#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LOG_2 0.30102999566

typedef struct{
	uint8_t a[1024];
} big_boi;


big_boi cast_int(void *a, uint8_t size){
	big_boi out;
	memset(&out, 0, sizeof(out));

	uint8_t *out_pointer = &out;

	uint64_t value;
	uint8_t *value_pointer = &value;
  
	switch(size){
	case 0:
		break;
	case 1:
		value = *((uint8_t *) a);
		break;
	case 2:
		value = *((uint16_t *) a);
		break;
	case 4:
		value = *((uint32_t *) a);
		break;
	case 8:
		value = *((uint64_t *) a);
		break;
	}

	for(uint8_t i = 0; i < size; ++i){
		*(out_pointer + i) = *(value_pointer + i);
	}

	return out;
}

uint8_t add_byte(uint8_t a, uint8_t b, uint8_t *c_in){
	uint16_t temp = ((uint16_t) a) + ((uint16_t) b) + ((uint16_t) *c_in);

	uint8_t out = temp;
 
	uint16_t temp_c_in = temp >> 8;
	*c_in = temp_c_in;

	return out;
}

big_boi multiply_boi(big_boi a, big_boi b){
	uint16_t temp = 0;

	uint8_t zero = 0;

	big_boi out = cast_int(&zero, 1);
	uint8_t *out_pointer = &out;

	big_boi temp_boi = cast_int(&zero, 1);

	uint8_t overflow = 0;

	uint8_t *a_pointer = &a;
	uint8_t *b_pointer = &b;

	for(uint16_t i = 0; i < 1023; ++i){
		for(uint16_t j = 0; i < 1023; ++i){
			temp = (*(a_pointer + j)) * (*(b_pointer + i));

			uint8_t left_byte = (temp & 0xFF00) >> 8;
			uint8_t right_byte = temp & 0x00FF;

			*(out_pointer + j) = add_byte(*(out_pointer + j), right_byte, &overflow);
			*(out_pointer + j + 1) = add_byte(*(out_pointer + j + 1), left_byte, &overflow);      
		}
	}

	return out;
}

big_boi add_boi(big_boi a, big_boi b){
	uint16_t temp = 0;

	uint8_t char_a;
	uint8_t char_b;

	uint8_t *a_pointer = &a;
	uint8_t *b_pointer = &b;

	big_boi out;

	uint8_t *out_pointer = &out;

	uint8_t overflow = 0;


	for(uint16_t i = 0; i < 1024; ++i){
		char_a = *(a_pointer + i);
		char_b = *(b_pointer + i);
    
		temp = add_byte(char_a, char_b, &overflow);

		*(out_pointer + i) = temp;
	}

	return out;
}


void left_shift(uint8_t *a, uint16_t size){

	uint8_t leftmost_bit = 0;
	uint8_t rightmost_bit = 0;

	for(uint16_t i = 0; i < size; ++i){
		//get the leftmost bit
		leftmost_bit = (*(a + i)) & 128;

		//shift left
		*(a + i) <<= 1;

		//put the leftmost bit from the
		//previous run of the loop into
		//rightmost spot
		*(a + i) |= rightmost_bit;

		//put leftmost_bit from loop into right most spot
		rightmost_bit = leftmost_bit >> 7;
	}
}

//convert big_boi to bcd
//double dabble algorithm/shift+3 algorithm
uint8_t *binary_to_bcd(big_boi a, uint16_t *array_size){

	//find most significant byte
	uint16_t most_significant = 1024;
	do{
		--most_significant;

		if(a.a[most_significant]){
			break;
		}

	} while(most_significant != 0);
  
	//number of decimal digits
	uint16_t number_of_digits = 8 * (most_significant + 1) * LOG_2 + 1;

	//each byte can store two digits
	*array_size = (number_of_digits + 1) / 2;

	uint8_t *bcd_array = calloc(*array_size, sizeof(*bcd_array));

	//counter for the dowhile
	uint16_t i = most_significant + 1;

	uint8_t bit = 0;

	//look at big_boi byte by byte, starting from
	//the most significant to the least
	do{
		--i;

		uint8_t byte = a.a[i];

		//loop over bits in a byte
		for(uint8_t j = 0; j < 8; ++j){

			//loop over all numbers in bcd_array
			//if the value is >= 5, add 3
			for(uint16_t k = 0; k <= (*array_size - 1); ++k){
				if((bcd_array[k] & 0x0F) >= 0x05){
					bcd_array[k] += 0x03;
				}

				if((bcd_array[k] & 0xF0) >= 0x50){
					bcd_array[k] += 0x30;
				}
			}


			//get the bit to shift into the bcd_array
			//it's either 0 or 1, relies on true being 0b00000001
			bit = !!(byte & 0x80);

			//left shift, put the bit into the first spot
			left_shift(bcd_array, *array_size);
			bcd_array[0] |= bit;

			byte <<= 1;

		}

	} while( i != 0);

	return bcd_array;
}


void print_boi(big_boi a){
	uint16_t array_size = 0;

	uint8_t *bcd_array = binary_to_bcd(a, &array_size);  

	uint8_t zero_flag = 0;
	for(uint16_t l = array_size; l < 1024; --l){
		uint8_t left_value = (bcd_array[l] & 0xF0) >> 4;
		uint8_t right_value = bcd_array[l] & 0X0F;

		//don't print leading zeros
		if(zero_flag || left_value){
			printf("%u", left_value);
			zero_flag = 1;
		}
		if(zero_flag || right_value){
			printf("%u", right_value);
			zero_flag = 1;
		}
	}

	free(bcd_array);
	return;
}


void main(void){
	uint8_t one = 1;
	uint8_t two = 2;

	uint16_t min = 1;
	uint16_t max = 64;
  
	big_boi a = cast_int(&one, 1);
	big_boi b = cast_int(&two, 1);

	for(uint64_t i = min; i <= max; ++i){
		a = multiply_boi(a, b);
		print_boi(a);
		printf("\n");
	}

	return;
}
