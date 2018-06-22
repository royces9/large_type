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
  uint16_t temp = ((uint16_t) a) + ((uint16_t) + b) + ((uint16_t) *c_in);

  uint8_t out = temp;
 
  uint16_t temp_c_in = temp >> 8;
  *c_in =  temp_c_in;

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


void print_boi(big_boi a){

  //find most significant byte
  uint16_t most_significant = 1024;
  do{
    --most_significant;

    if(a.a[most_significant]){
      break;
    }

  } while(most_significant != 0);

  //convert to bcd
  uint16_t number_of_digits = 8 * (most_significant + 1) * LOG_2 + 1;

  //each byte can store two digits
  uint16_t array_size = (number_of_digits + 1) / 2;

  uint8_t *bcd_array = calloc(array_size, sizeof(*bcd_array));
  {
    uint16_t i = most_significant + 1;

    do{
      --i;


      uint8_t byte = a.a[i];
      uint8_t bit = 0;
      for(uint8_t j = 0; j < 8; ++j){

	for(uint16_t k = 0; k <= (most_significant - i); ++k){
	  uint8_t left_half = (bcd_array[k] >> 4) & 15;
	  uint8_t right_half = bcd_array[k] & 15;
	  if(left_half >= 5){
	    left_half += 3;
	  }
	  if(right_half >= 5){
	    right_half += 3;
	  }

	  bcd_array[k] = (left_half << 4) | right_half;
	}


	//get the bit to shift into the bcd_array
	bit = byte & 128;
	bit >>= 7;

	//left shift, put the bit into the first spot
	left_shift(bcd_array, array_size);
	bcd_array[0] |= bit;

	byte <<= 1;

      }

    } while( i != 0);
  }

  uint8_t zero_flag = 0;
  for(uint16_t l = array_size; l < 1024; --l){
    if(zero_flag || ((bcd_array[l] >> 4)) & 15){
      printf("%u", (bcd_array[l] >> 4) & 15);
      zero_flag = 1;
    }
    if(zero_flag || (bcd_array[l] & 15)){
      printf("%u", bcd_array[l] & 15);
      zero_flag = 1;
    }



  }

  return;
}

void main(void){
  uint64_t a = 1;
  for(uint64_t i = 0; i <= 64; ++i){
    printf("%u = ", i);
    print_boi(cast_int(&a, 8));
    printf("\n");
    a *= 2;
  }

  return;
}
