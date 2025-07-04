#ifndef __WS2812_H
#define __WS2812B_H

__HIGH_CODE
void WS2812BSimpleSend(uint32_t pin, uint8_t * data, int len_in_bytes )
{

	GPIOA_ResetBits(pin);

	uint8_t * end = data + len_in_bytes;
	while( data != end )
	{
		uint8_t byte = *data;

		int i;
		for( i = 0; i < 8; i++ )
		{
			if( byte & 0x80 )
			{
				// WS2812B's need AT LEAST 625ns for a logical "1"
				GPIOA_SetBits(pin);
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );

				GPIOA_ResetBits(pin);
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );

			}
			else
			{
				// WS2812B's need BETWEEN 62.5 to about 500 ns for a logical "0"
				GPIOA_SetBits(pin);
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );

				GPIOA_ResetBits(pin);
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
				asm volatile( "nop\nnop\nnop\nnop\nnop\nnop" );
			}
			byte <<= 1;
		}

		data++;
	}

	GPIOA_ResetBits(pin);
}

#endif
