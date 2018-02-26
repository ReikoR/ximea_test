__constant sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;

__kernel void debayer (
	__global uchar* input,
	__global uchar* output,
	int width,
    int height
) {
    int x = get_global_id(0) * 2;
    int y = get_global_id(1) * 2;

    int xy = y * width + x;
    int txy = xy * 3;

    //output[txy] = input[xy + width + 1];
    //output[txy + 1] = input[xy+1];
    //output[txy + 2] = input[xy];

    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B

    uchar left = x > 0 ? input[xy-1] : 0;
    uchar right = input[xy+1];
    uchar up = y > 0 ? input[xy-width] : 0;
    uchar down = input[xy+width];
    uchar upLeft = y > 0 && x > 0 ? input[xy-width-1] : 0;
    uchar upRight = y > 0 ? input[xy-width+1] : 0;
    uchar downLeft = x > 0 ? input[xy+width-1] : 0;
    uchar downRight = input[xy+width+1];

    output[txy++] = (upLeft + upRight + downLeft + downRight + 2) >> 2; //diagonal
    output[txy++] = (left + right + up + down + 2) >> 2; //left,right,up,down
    output[txy++] = input[xy];

    /*xy += 1;
    output[txy++] = (input[xy-w]+input[xy+w]+1) >> 1;//up,down    
    output[txy++] = input[xy];
    output[txy++] = (input[xy-1]+input[xy+1]+1) >> 1;//left,right

    xy += w - 1;
    txy = xy * 3;
    output[txy++] = (input[xy-1]+input[xy+1]+1) >> 1;//left,right    
    output[txy++] = input[xy];
    output[txy++] = (input[xy-w] + input[xy+w]+1) >> 1;//up,down

    xy += 1;
    output[txy]   = input[xy];    
    output[txy++] = (input[xy-1]+input[xy+1]+input[xy-w]+input[xy+w]+2) >> 2;//left,right,up,down
    output[txy++] = (input[xy-w-1]+input[xy-w+1]+input[xy+w-1]+input[xy+w+1]+2) >> 2;//diagonal
    */
}

__kernel void debayer2 (
	__global uchar* input,
	__global uchar* output,
	int width,
    int height
) {
    int x = get_global_id(0) * 2;
    int y = get_global_id(1) * 2;

    int xy = y * width + x;
    int txy = xy * 3;

    output[txy] = input[xy + width + 1];
    output[txy + 1] = input[xy + 1];
    output[txy + 2] = input[xy];
}

__kernel void debayer3(
    __global uchar* input,
    __global uchar* output,
    int width,
    int height
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    // 4 pixels per call, 2 pixels of 2 lines
    int sourcePixelIndex = (2 * y) * width + 2 * x - width;

    int destWidth = 2 * get_global_size(0);
    int destY     = 2 * y;
    int destX     = 2 * x;

    uchar4 line_0;
    uchar4 line_1;
    uchar4 line_2;
    uchar4 line_3;

    // vector access .x .y .z. w
    line_0.x = input[sourcePixelIndex-1];
    line_0.y = input[sourcePixelIndex];
    line_0.z = input[sourcePixelIndex+1];
    line_0.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_1.x = input[sourcePixelIndex-1];
    line_1.y = input[sourcePixelIndex];
    line_1.z = input[sourcePixelIndex+1];
    line_1.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_2.x = input[sourcePixelIndex-1];
    line_2.y = input[sourcePixelIndex];
    line_2.z = input[sourcePixelIndex+1];
    line_2.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_3.x = input[sourcePixelIndex-1];
    line_3.y = input[sourcePixelIndex];
    line_3.z = input[sourcePixelIndex+1];
    line_3.w = input[sourcePixelIndex+2];

    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B

    // first pixel first line
    ushort blue_00  = (line_0.x + line_0.z + line_2.x + line_2.z) / 4;
    ushort green_00 = (line_0.y + line_1.x + line_1.z + line_2.y) / 4;
    ushort red_00   = line_1.y;

    // second pixel first line
    ushort blue_01   = hadd(line_0.z, line_2.z);
    ushort green_01  = line_1.z;
    ushort red_01    = hadd(line_1.y, line_1.w);

    // first pixel second line
    ushort blue_10   = hadd(line_2.x, line_2.z);
    ushort green_10  = line_2.y;
    ushort red_10    = hadd(line_1.y, line_3.y);

    // second pixel second line
    ushort blue_11   = line_2.z;
    ushort green_11  = (line_1.z + line_2.y + line_2.w + line_3.z) / 4;
    ushort red_11    = (line_1.y + line_1.w + line_3.y + line_3.w) / 4;

    // first pixel first line
    int destPixelIndex = (destY * destWidth + destX) * 3;
    output[destPixelIndex]    = blue_00;
    output[destPixelIndex+1]  = green_00;
    output[destPixelIndex+2]  = red_00;

    // second pixel first line
    output[destPixelIndex+3]  = blue_01;
    output[destPixelIndex+4]  = green_01;
    output[destPixelIndex+5]  = red_01;

    // first pixel second line
    destPixelIndex += destWidth * 3;
    output[destPixelIndex]    = blue_10;
    output[destPixelIndex+1]  = green_10;
    output[destPixelIndex+2]  = red_10;

    // second pixel second line
    output[destPixelIndex+3]  = blue_11;
    output[destPixelIndex+4]  = green_11;
    output[destPixelIndex+5]  = red_11;
}

__kernel void debayerAndSegment(
    __global uchar* input,
    __global uchar* output,
    __global uchar* lookup,
    __global uchar* segmented,
    int width,
    int height
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    // 4 pixels per call, 2 pixels of 2 lines
    int sourcePixelIndex = (2 * y) * width + 2 * x - width;

    int destWidth = 2 * get_global_size(0);
    int destY     = 2 * y;
    int destX     = 2 * x;
    int xy = destY * width + destX;

    uchar4 line_0;
    uchar4 line_1;
    uchar4 line_2;
    uchar4 line_3;

    // vector access .x .y .z. w
    line_0.x = input[sourcePixelIndex-1];
    line_0.y = input[sourcePixelIndex];
    line_0.z = input[sourcePixelIndex+1];
    line_0.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_1.x = input[sourcePixelIndex-1];
    line_1.y = input[sourcePixelIndex];
    line_1.z = input[sourcePixelIndex+1];
    line_1.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_2.x = input[sourcePixelIndex-1];
    line_2.y = input[sourcePixelIndex];
    line_2.z = input[sourcePixelIndex+1];
    line_2.w = input[sourcePixelIndex+2];

    sourcePixelIndex += width;
    line_3.x = input[sourcePixelIndex-1];
    line_3.y = input[sourcePixelIndex];
    line_3.z = input[sourcePixelIndex+1];
    line_3.w = input[sourcePixelIndex+2];

    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B
    //R G R G R G
    //G B G B G B

    // first pixel first line
    ushort blue_00  = (line_0.x + line_0.z + line_2.x + line_2.z) / 4;
    ushort green_00 = (line_0.y + line_1.x + line_1.z + line_2.y) / 4;
    ushort red_00   = line_1.y;

    // second pixel first line
    ushort blue_01   = hadd(line_0.z, line_2.z);
    ushort green_01  = line_1.z;
    ushort red_01    = hadd(line_1.y, line_1.w);

    // first pixel second line
    ushort blue_10   = hadd(line_2.x, line_2.z);
    ushort green_10  = line_2.y;
    ushort red_10    = hadd(line_1.y, line_3.y);

    // second pixel second line
    ushort blue_11   = line_2.z;
    ushort green_11  = (line_1.z + line_2.y + line_2.w + line_3.z) / 4;
    ushort red_11    = (line_1.y + line_1.w + line_3.y + line_3.w) / 4;

    // first pixel first line
    int destPixelIndex = (destY * destWidth + destX) * 3;
    output[destPixelIndex]    = blue_00;
    output[destPixelIndex+1]  = green_00;
    output[destPixelIndex+2]  = red_00;
    segmented[xy] = lookup[blue_00 + (green_00 << 8) + (red_00 << 16)];

    // second pixel first line
    output[destPixelIndex+3]  = blue_01;
    output[destPixelIndex+4]  = green_01;
    output[destPixelIndex+5]  = red_01;
    segmented[xy + 1] = lookup[blue_01 + (green_01 << 8) + (red_01 << 16)];

    // first pixel second line
    destPixelIndex += destWidth * 3;
    output[destPixelIndex]    = blue_10;
    output[destPixelIndex+1]  = green_10;
    output[destPixelIndex+2]  = red_10;
    xy += width;
    segmented[xy] = lookup[blue_10 + (green_10 << 8) + (red_10 << 16)];

    // second pixel second line
    output[destPixelIndex+3]  = blue_11;
    output[destPixelIndex+4]  = green_11;
    output[destPixelIndex+5]  = red_11;
    segmented[xy + 1] = lookup[blue_11 + (green_11 << 8) + (red_11 << 16)];
}