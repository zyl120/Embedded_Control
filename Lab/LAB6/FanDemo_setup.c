//Function to use ranger to adjust the thruster fan angle
//holding a fixed range value for ~5 seconds locks in the corresponding angle.
void Set_Angle(void)
{
    char set_angle, count;
    unsigned int adj, previous_adj, angle;

    count = 0;
    previous_adj = 0;
    set_angle = 1;

    while(set_angle)
    {
        while(!new_range);                          //new_range is global 80ms flag
	new_range = 0;
        adj = Read_Ranger();
        angle = 2765 + (adj - 40)*10;               //May change: 40 nominal height
                                                    //            10 gain on rotation
        if (angle < PWMIN) angle = PWMIN;
        else if (angle > PWMAX) angle = PWMAX;
        PCA0CP1 = 0xFFFF - angle;                   //CEX1 is thrust fan servo

        if(abs(previous_adj - adj) < 8) count++;    //Adjust depending on how noisy data is
	else count = 0;
        if(count > 62) set_angle = 0;               //Assuming ranger reads every 80ms
	previous_adj = adj;                         //
	printf("\r Range = %u   ", adj);
    }
}



unsigned int Read_Ranger(void)
{
    unsigned int range;

    i2c_read_data(0xE0, 2, Data, 2);
    range = (Data[0] << 8) + Data[1];
    Data[0] = 0x51;
    i2c_write_data(0xE0, 0, Data, 1);   // ping for next read range
    return range;
}