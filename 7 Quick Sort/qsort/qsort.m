X = [100 200 300 400 500 750 1000 1250 1500];


Y_pro_on_FLASH_data_on_FLASH = [1.187 2.692 4.390 5.978 7.4 11.819 16.064 20.066 24.011];

Y_pro_on_FLASH_data_on_RAM   = [1.000 2.272 3.828 5.224 6.463 10.232 14.190 17.426 20.844];

Y_pro_on_CCM_data_on_FLASH   = [1.157 2.614 4.257 5.795 7.174 11.398 15.573 19.374 23.183];

Y_pro_on_CCM_data_on_RAM     = [0.970 2.197 3.700 5.047 6.244 9.821 13.713 16.752 20.037]
hold on

plot(X,Y_pro_on_FLASH_data_on_FLASH,"*");
plot(X,Y_pro_on_FLASH_data_on_RAM,"*");
plot(X,Y_pro_on_CCM_data_on_FLASH,"*");
plot(X,Y_pro_on_CCM_data_on_RAM,"*");


legend('pro_on_FLASH_data_on_FLASH', 'Y_pro_on_FLASH_data_on_RAM', 'Y_pro_on_CCM_data_on_FLASH', 'Y_pro_on_CCM_data_on_RAM')
hold off