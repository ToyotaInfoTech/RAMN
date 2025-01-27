uint8_t FLAG_1[] = "flag{REDACTED_FLAG_1}";
uint8_t flag1_written = 0U;
uint8_t log_written = 0U;
uint8_t read_buf[512];
uint8_t write_buf[512];

void write_flag_1()
{
	FATFS FatFs;
	FIL fil;
	FRESULT fres;
	UINT bytes_written = 0U;

	fres = f_mount(&FatFs, "", 1); //mount drive
	if (fres == FR_OK)
	{
		fres = f_open(&fil, "flag1.txt", FA_WRITE);
		if (fres == FR_OK)
		{
			fres = f_write(&fil,FLAG_1, sizeof(FLAG_1)-1, &bytes_written);
			if (fres == FR_OK)
			{
				if (bytes_written == sizeof(FLAG_1)-1) flag1_written = 1U;
			}
			f_sync(&fil);
			f_close(&fil);
		}
		f_mount(0, "", 0); //reset data
	}
}


uint8_t validate_config_file()
{
	FATFS FatFs;
	FIL fil;
	FRESULT fres;
	UINT bytes_read = 0U;
	uint8_t valid_file = 0U;

	fres = f_mount(&FatFs, "", 1); //note that this won't reinitialize the SD CARD if it has already been initialized.
	if (fres == FR_OK)
	{
		fres = f_open(&fil, "config.txt", FA_READ);
		if (fres == FR_OK)
		{
			FSIZE_t file_size = f_size(&fil);
			if (file_size < (sizeof(read_buf)-1))
			{
				fres = f_read(&fil,read_buf,file_size,&bytes_read);
				if (fres == FR_OK)
				{
					if (bytes_read == file_size)
					{
						valid_file = 1U;
						for (uint32_t i = 0; i < file_size; i++)
						{
							if (read_buf[i] == '%')
							{
								valid_file = 0U;
								break;
							}
						}
					}
				}
			}
			f_close(&fil);
		}
		f_mount(0, "", 0); //reset data
	}
	return valid_file;
}

uint8_t header_message[] = "STARTING LOG WITH CONFIG: ";
void write_log_header()
{
	FATFS FatFs;
	FIL fil;
	FRESULT fres;
	UINT bytes_read = 0U;
	UINT bytes_written = 0U;
	uint8_t read_ok = 0U;
	volatile uint8_t FLAG_2[] = "flag{REDACTED_FLAG_2}";


	fres = f_mount(&FatFs, "", 1); //note that this won't reinitialize the SD CARD if it has already been initialized.
	if (fres == FR_OK)
	{
		fres = f_open(&fil, "config.txt", FA_READ);
		if (fres == FR_OK)
		{
			FSIZE_t file_size = f_size(&fil);
			if (file_size < (sizeof(read_buf)-1))
			{
				fres = f_read(&fil,read_buf,file_size,&bytes_read);
				if (fres == FR_OK)
				{
					if (bytes_read == file_size)
					{
						read_ok = 1U;
					}
				}
			}
			f_close(&fil);
		}

		if (read_ok != 0U)
		{
			fres = f_open(&fil, "log.txt", FA_WRITE);
			if (fres == FR_OK) {
				UINT index = 0U;
				index = snprintf(write_buf, sizeof(write_buf)-1, header_message);
				index += snprintf(write_buf+index, sizeof(write_buf)-(index+1), read_buf);
				fres = f_write(&fil, write_buf, index, &bytes_written);
				if (fres == FR_OK)
				{
					if(bytes_written == index)
					{
						log_written = 1U;
					}
				}
				f_sync(&fil);
				f_close(&fil);
			}
		}
		f_mount(0, "", 0); //reset data
	}
}


/* ... */

void RAMN_SDCardTaskFunc(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
	osDelay(500); //leave time for hardware to start
	for(;;)
	{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();

		//Use 9765Hz clock to read SD CARD data over SPI
		//Whole operation may take up to ~18 seconds
		//Other activities are interrupted while reading
		
		//Try to write flag 1 if it has never been written before
		if (flag1_written == 0U)
		{
			write_flag_1();
		}

		//Write log header if sanity check is OK
		if (log_written == 0U)
		{
			if (validate_config_file() != 0U)
			{
				write_log_header();
			}
		}

		//Make sure CS is released
		HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

		vTaskDelayUntil(&xLastWakeTime, 60000);
	}
  /* USER CODE END 5 */
}