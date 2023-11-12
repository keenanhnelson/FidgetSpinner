#include "pixels_spi_wrapper.h"
#include "spi.h"

//Wrap platform dependent spi code goes here

void sendSpiPixelDataWrapper(uint8_t *data, uint32_t len){
	HAL_SPI_Transmit(&hspi1, data, len, 1000);
}
