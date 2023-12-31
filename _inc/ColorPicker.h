
#pragma once

#define _PIXELCOLORPICKER_INCLUDED_ 1

// image of the cursor (32x32 pixels)
// Each line in the code represents line of the cursor.
// To minimize memory footprint, the pixels are encoded as follows:
//  - first byte of each line is the number of empty pixels on that line
//  - second byte is the number of valid pixels on that line (sum of 1st and 2nd byte <= 32)
//  - following 3*(2nd byte) bytes represent pixels
//     - each pixel is described by 3 bytes: 1st byte is opacity, 2nd brightness, 3rd the amount of colorization
// To create your own cursors comfortably, read the instructions on http://www.rw-designer.com/color-picker

//extern __declspec(selectany) BYTE const g_aPattern[] = // the image is upside down
//{
//	0x04, 0x06, 0x04, 0x01, 0x01, 0x10, 0x02, 0x02, 0x10, 0x02, 0x02, 0x0a, 0x01, 0x01, 0x05, 0x00, 0x00, 0x01, 0x00, 0x00, 
//	0x03, 0x0a, 0x07, 0x02, 0x02, 0x52, 0x0b, 0x0b, 0xab, 0x1b, 0x1b, 0xb5, 0x0d, 0x0d, 0xa5, 0x08, 0x08, 0x88, 0x02, 0x02, 0x63, 0x03, 0x03, 0x3d, 0x03, 0x03, 0x1a, 0x03, 0x03, 0x04, 0x01, 0x01, 
//	0x03, 0x0b, 0x23, 0x04, 0x04, 0xd7, 0x50, 0x38, 0xff, 0x59, 0x31, 0xff, 0x54, 0x2c, 0xff, 0x53, 0x2b, 0xff, 0x44, 0x26, 0xfa, 0x38, 0x22, 0xea, 0x25, 0x1d, 0xb5, 0x13, 0x13, 0x4d, 0x05, 0x05, 0x0a, 0x02, 0x02, 
//	0x03, 0x0c, 0x49, 0x16, 0x16, 0xf8, 0x76, 0x42, 0xff, 0x79, 0x3a, 0xff, 0x6c, 0x35, 0xff, 0x64, 0x32, 0xff, 0x5e, 0x2f, 0xff, 0x5b, 0x2c, 0xff, 0x58, 0x2c, 0xfe, 0x48, 0x2c, 0xdf, 0x1b, 0x1a, 0x59, 0x06, 0x06, 0x09, 0x02, 0x02, 
//	0x02, 0x0e, 0x02, 0x01, 0x01, 0x6c, 0x28, 0x28, 0xfe, 0x94, 0x46, 0xff, 0x91, 0x45, 0xff, 0x8c, 0x42, 0xff, 0x86, 0x40, 0xff, 0x7d, 0x3e, 0xff, 0x71, 0x3a, 0xff, 0x64, 0x34, 0xff, 0x5b, 0x2d, 0xff, 0x49, 0x2d, 0xd6, 0x1e, 0x1e, 0x3b, 0x06, 0x06, 0x01, 0x00, 0x00, 
//	0x02, 0x0e, 0x05, 0x02, 0x02, 0x8f, 0x47, 0x40, 0xfe, 0xa5, 0x4e, 0xff, 0xaa, 0x4e, 0xff, 0xaa, 0x4f, 0xff, 0xa4, 0x4f, 0xff, 0x9b, 0x4e, 0xff, 0x8d, 0x49, 0xff, 0x79, 0x3f, 0xff, 0x63, 0x34, 0xff, 0x5a, 0x2c, 0xfe, 0x3a, 0x2c, 0x91, 0x0d, 0x0d, 0x0f, 0x03, 0x03, 
//	0x02, 0x0f, 0x05, 0x02, 0x02, 0x97, 0x5f, 0x41, 0xff, 0xba, 0x56, 0xff, 0xc3, 0x5a, 0xff, 0xc7, 0x5f, 0xff, 0xc6, 0x65, 0xff, 0xb5, 0x5c, 0xff, 0xa0, 0x51, 0xff, 0x8b, 0x46, 0xff, 0x71, 0x3b, 0xff, 0x5e, 0x2f, 0xff, 0x51, 0x2c, 0xdc, 0x1d, 0x1d, 0x35, 0x06, 0x06, 0x01, 0x00, 0x00, 
//	0x02, 0x0f, 0x05, 0x03, 0x03, 0x90, 0x5e, 0x4a, 0xfe, 0xcb, 0x5f, 0xff, 0xdd, 0x69, 0xff, 0xef, 0x81, 0xff, 0xf3, 0x89, 0xff, 0xd2, 0x6e, 0xff, 0xb4, 0x5b, 0xff, 0x9d, 0x4e, 0xff, 0x86, 0x43, 0xff, 0x6b, 0x36, 0xff, 0x5a, 0x2c, 0xfc, 0x35, 0x2a, 0x83, 0x0c, 0x0c, 0x10, 0x03, 0x03, 
//	0x02, 0x10, 0x04, 0x03, 0x03, 0x80, 0x5b, 0x5b, 0xfd, 0xcd, 0x69, 0xff, 0xf6, 0x85, 0xff, 0xfe, 0xa6, 0xff, 0xff, 0xa2, 0xff, 0xf7, 0x94, 0xff, 0xd2, 0x6d, 0xff, 0xb0, 0x57, 0xff, 0x96, 0x4a, 0xff, 0x7c, 0x3e, 0xff, 0x61, 0x31, 0xff, 0x48, 0x2e, 0xd7, 0x26, 0x26, 0x46, 0x09, 0x09, 0x04, 0x01, 0x01, 
//	0x02, 0x10, 0x01, 0x01, 0x01, 0x57, 0x5c, 0x5c, 0xf0, 0xab, 0x7a, 0xff, 0xfc, 0x86, 0xff, 0xff, 0xa3, 0xff, 0xfa, 0xb2, 0xff, 0xf7, 0xa9, 0xff, 0xef, 0x87, 0xff, 0xba, 0x62, 0xff, 0xb7, 0x52, 0xff, 0xa7, 0x46, 0xff, 0x97, 0x39, 0xff, 0x6e, 0x2f, 0xf6, 0x34, 0x34, 0x92, 0x1c, 0x1c, 0x25, 0x05, 0x05, 
//	0x03, 0x10, 0x13, 0x09, 0x09, 0xad, 0x88, 0x87, 0xfa, 0xc7, 0x86, 0xff, 0xf1, 0x88, 0xff, 0xfa, 0xa3, 0xff, 0xff, 0xa5, 0xff, 0xfa, 0x90, 0xff, 0xdf, 0x74, 0xff, 0xc2, 0x5d, 0xfe, 0xa1, 0x51, 0xfc, 0x7d, 0x47, 0xee, 0x49, 0x3f, 0xd0, 0x3b, 0x3b, 0xbf, 0x36, 0x36, 0x67, 0x0f, 0x0f, 0x15, 0x04, 0x04, 
//	0x03, 0x11, 0x01, 0x02, 0x02, 0x33, 0x54, 0x54, 0xc3, 0x92, 0x92, 0xf6, 0xa1, 0x93, 0xfe, 0xb6, 0x9e, 0xfc, 0xd2, 0xb2, 0xf7, 0xc3, 0xae, 0xef, 0xae, 0x9e, 0xe6, 0x89, 0x83, 0xd6, 0x70, 0x70, 0xc0, 0x66, 0x66, 0xa9, 0x5d, 0x5d, 0x97, 0x51, 0x51, 0xad, 0x42, 0x42, 0xb1, 0x31, 0x31, 0x53, 0x0a, 0x0a, 0x0b, 0x02, 0x02, 
//	0x04, 0x11, 0x03, 0x04, 0x04, 0x37, 0x4b, 0x4b, 0xb4, 0x8a, 0x8a, 0xe0, 0x9e, 0x9e, 0xd7, 0xaf, 0xaf, 0xc1, 0xd4, 0xd4, 0xa9, 0xdb, 0xdb, 0x9d, 0xc7, 0xc7, 0x96, 0xa6, 0xa6, 0x90, 0x91, 0x91, 0x8c, 0x7e, 0x7e, 0x8b, 0x69, 0x69, 0x93, 0x50, 0x50, 0xb7, 0x3f, 0x3f, 0xa0, 0x27, 0x27, 0x3f, 0x08, 0x08, 0x07, 0x01, 0x01, 
//	0x05, 0x12, 0x02, 0x02, 0x02, 0x21, 0x2c, 0x2c, 0x8d, 0x82, 0x82, 0xc6, 0xa4, 0xa4, 0xb3, 0xc1, 0xc1, 0x9b, 0xf4, 0xf4, 0x8c, 0xf2, 0xf2, 0x8b, 0xd4, 0xd4, 0x8b, 0xab, 0xab, 0x8b, 0x90, 0x90, 0x8b, 0x79, 0x79, 0x8e, 0x61, 0x61, 0xab, 0x44, 0x44, 0xdd, 0x38, 0x38, 0xbd, 0x19, 0x19, 0x81, 0x02, 0x02, 0x48, 0x00, 0x00, 0x0c, 0x00, 0x00, 
//	0x07, 0x11, 0x12, 0x0a, 0x0a, 0x8a, 0x88, 0x88, 0xc5, 0xa3, 0xa3, 0xac, 0xc5, 0xc5, 0x94, 0xf5, 0xf5, 0x8c, 0xec, 0xec, 0x8b, 0xcb, 0xcb, 0x8b, 0xa2, 0xa2, 0x93, 0x81, 0x81, 0xaf, 0x5d, 0x5d, 0xde, 0x3b, 0x3b, 0xfd, 0x37, 0x37, 0xff, 0x3d, 0x3d, 0xfe, 0x3b, 0x3b, 0xe8, 0x1c, 0x1c, 0x59, 0x00, 0x00, 0x02, 0x00, 0x00, 
//	0x08, 0x10, 0x1d, 0x2e, 0x2e, 0x9c, 0x90, 0x90, 0xc3, 0xa4, 0xa4, 0xa8, 0xc8, 0xc8, 0x8e, 0xee, 0xee, 0x8b, 0xe4, 0xe4, 0x98, 0xb0, 0xb0, 0xc0, 0x74, 0x74, 0xef, 0x42, 0x42, 0xfe, 0x3f, 0x3f, 0xff, 0x4f, 0x4f, 0xff, 0x52, 0x52, 0xff, 0x4c, 0x4c, 0xff, 0x46, 0x46, 0xc4, 0x05, 0x05, 0x1e, 0x00, 0x00, 
//	0x08, 0x11, 0x01, 0x02, 0x02, 0x23, 0x2e, 0x2e, 0xab, 0x95, 0x95, 0xc2, 0xa6, 0xa6, 0xa1, 0xc7, 0xc7, 0x9a, 0xcf, 0xcf, 0xc8, 0xa0, 0xa0, 0xf8, 0x58, 0x58, 0xff, 0x52, 0x52, 0xff, 0x6e, 0x6e, 0xff, 0x74, 0x74, 0xff, 0x6e, 0x6e, 0xff, 0x51, 0x51, 0xff, 0x4c, 0x4c, 0xfb, 0x28, 0x28, 0x84, 0x00, 0x00, 0x0e, 0x00, 0x00, 
//	0x09, 0x11, 0x02, 0x03, 0x03, 0x39, 0x4d, 0x4d, 0xbb, 0x99, 0x99, 0xc5, 0xa4, 0xa4, 0xd2, 0x9a, 0x9a, 0xf9, 0x5d, 0x5d, 0xff, 0x6b, 0x6b, 0xff, 0x8d, 0x8d, 0xff, 0x93, 0x93, 0xff, 0x8c, 0x8c, 0xff, 0x77, 0x77, 0xff, 0x5a, 0x5a, 0xff, 0x4c, 0x4c, 0xff, 0x4a, 0x4a, 0xf2, 0x19, 0x19, 0x81, 0x00, 0x00, 0x17, 0x00, 0x00, 
//	0x0a, 0x11, 0x05, 0x04, 0x04, 0x53, 0x65, 0x65, 0xd3, 0x90, 0x90, 0xfd, 0x6a, 0x6a, 0xff, 0x7c, 0x7c, 0xff, 0xa6, 0xa6, 0xff, 0xb1, 0xb1, 0xff, 0xa5, 0xa5, 0xff, 0x8f, 0x8f, 0xff, 0x77, 0x77, 0xff, 0x61, 0x61, 0xff, 0x4f, 0x4f, 0xff, 0x4c, 0x4c, 0xff, 0x4a, 0x4a, 0xf8, 0x27, 0x27, 0x98, 0x00, 0x00, 0x1b, 0x00, 0x00, 
//	0x0b, 0x11, 0x0e, 0x04, 0x04, 0xad, 0x45, 0x45, 0xff, 0x81, 0x81, 0xff, 0xb8, 0xb8, 0xff, 0xc8, 0xc8, 0xff, 0xbe, 0xbe, 0xff, 0xa6, 0xa6, 0xff, 0x90, 0x90, 0xff, 0x7e, 0x7e, 0xff, 0x6c, 0x6c, 0xff, 0x59, 0x59, 0xff, 0x4c, 0x4c, 0xff, 0x4c, 0x4c, 0xff, 0x4c, 0x4c, 0xfa, 0x29, 0x29, 0x9a, 0x00, 0x00, 0x19, 0x00, 0x00, 
//	0x0b, 0x12, 0x0a, 0x00, 0x00, 0xb0, 0x5f, 0x5f, 0xff, 0xba, 0xba, 0xff, 0xcf, 0xcf, 0xff, 0xcf, 0xcf, 0xff, 0xbb, 0xbb, 0xff, 0xa7, 0xa7, 0xff, 0x95, 0x95, 0xff, 0x84, 0x84, 0xff, 0x75, 0x75, 0xff, 0x64, 0x64, 0xff, 0x56, 0x56, 0xff, 0x4d, 0x4d, 0xff, 0x4c, 0x4c, 0xff, 0x4c, 0x4c, 0xf9, 0x2a, 0x2a, 0x82, 0x00, 0x00, 0x09, 0x00, 0x00, 
//	0x0b, 0x12, 0x03, 0x00, 0x00, 0x79, 0x6f, 0x6f, 0xfd, 0xc5, 0xc5, 0xff, 0xcd, 0xcd, 0xff, 0xc5, 0xc5, 0xff, 0xb9, 0xb9, 0xff, 0xa9, 0xa9, 0xff, 0x9b, 0x9b, 0xff, 0x8b, 0x8b, 0xff, 0x7e, 0x7e, 0xff, 0x74, 0x74, 0xff, 0x68, 0x68, 0xff, 0x5c, 0x5c, 0xff, 0x50, 0x50, 0xff, 0x4c, 0x4c, 0xff, 0x4c, 0x4c, 0xe3, 0x13, 0x13, 0x37, 0x00, 0x00, 
//	0x0c, 0x12, 0x0f, 0x00, 0x00, 0x78, 0x5d, 0x5d, 0xf8, 0xbd, 0xbd, 0xff, 0xbf, 0xbf, 0xff, 0xb7, 0xb7, 0xff, 0xab, 0xab, 0xff, 0x9e, 0x9e, 0xff, 0x94, 0x94, 0xff, 0x8b, 0x8b, 0xff, 0x83, 0x83, 0xff, 0x7a, 0x7a, 0xff, 0x6e, 0x6e, 0xff, 0x62, 0x62, 0xff, 0x50, 0x50, 0xff, 0x4c, 0x4c, 0xfe, 0x37, 0x37, 0x8c, 0x00, 0x00, 0x08, 0x00, 0x00, 
//	0x0d, 0x11, 0x09, 0x00, 0x00, 0x85, 0x64, 0x64, 0xfd, 0xb9, 0xb9, 0xff, 0xb5, 0xb5, 0xff, 0xad, 0xad, 0xff, 0xa5, 0xa5, 0xff, 0xa1, 0xa1, 0xff, 0x9b, 0x9b, 0xff, 0x92, 0x92, 0xff, 0x89, 0x89, 0xff, 0x7f, 0x7f, 0xff, 0x73, 0x73, 0xff, 0x63, 0x63, 0xff, 0x4c, 0x4c, 0xff, 0x4b, 0x4b, 0xb9, 0x10, 0x10, 0x12, 0x00, 0x00, 
//	0x0e, 0x10, 0x1c, 0x00, 0x00, 0xcf, 0x92, 0x92, 0xff, 0xb3, 0xb3, 0xff, 0xb2, 0xb2, 0xff, 0xb2, 0xb2, 0xff, 0xae, 0xae, 0xff, 0xab, 0xab, 0xff, 0xa3, 0xa3, 0xff, 0x98, 0x98, 0xff, 0x8e, 0x8e, 0xff, 0x83, 0x83, 0xff, 0x76, 0x76, 0xff, 0x62, 0x62, 0xff, 0x50, 0x50, 0xae, 0x04, 0x04, 0x0c, 0x00, 0x00, 
//	0x0e, 0x10, 0x02, 0x00, 0x00, 0x66, 0x4f, 0x4f, 0xfc, 0xb1, 0xb1, 0xff, 0xb9, 0xb9, 0xff, 0xbc, 0xbc, 0xff, 0xbd, 0xbd, 0xff, 0xba, 0xba, 0xff, 0xb4, 0xb4, 0xff, 0xa8, 0xa8, 0xff, 0x9c, 0x9c, 0xff, 0x92, 0x92, 0xff, 0x86, 0x86, 0xff, 0x77, 0x77, 0xfe, 0x61, 0x61, 0x8a, 0x00, 0x00, 0x07, 0x00, 0x00, 
//	0x0f, 0x0f, 0x12, 0x00, 0x00, 0xb2, 0x82, 0x82, 0xff, 0xbe, 0xbe, 0xff, 0xc3, 0xc3, 0xff, 0xca, 0xca, 0xff, 0xc8, 0xc8, 0xff, 0xc2, 0xc2, 0xff, 0xbb, 0xbb, 0xff, 0xac, 0xac, 0xff, 0x9c, 0x9c, 0xff, 0x91, 0x91, 0xff, 0x85, 0x85, 0xde, 0x40, 0x40, 0x3b, 0x00, 0x00, 0x01, 0x00, 0x00, 
//	0x10, 0x0d, 0x38, 0x24, 0x24, 0xed, 0xb4, 0xb4, 0xff, 0xc8, 0xc8, 0xff, 0xcc, 0xcc, 0xff, 0xd3, 0xd3, 0xff, 0xd0, 0xd0, 0xff, 0xc5, 0xc5, 0xff, 0xb2, 0xb2, 0xff, 0xa2, 0xa2, 0xff, 0x97, 0x97, 0xee, 0x5e, 0x5e, 0x5e, 0x03, 0x03, 0x05, 0x00, 0x00, 
//	0x10, 0x0c, 0x05, 0x00, 0x00, 0x5c, 0x4e, 0x4e, 0xf1, 0xbc, 0xbc, 0xff, 0xce, 0xce, 0xff, 0xd3, 0xd3, 0xff, 0xd2, 0xd2, 0xff, 0xbf, 0xbf, 0xff, 0xb2, 0xb2, 0xff, 0xa5, 0xa5, 0xee, 0x7d, 0x7d, 0x75, 0x09, 0x09, 0x0a, 0x00, 0x00, 
//	0x11, 0x0a, 0x06, 0x00, 0x00, 0x5e, 0x51, 0x51, 0xef, 0xc2, 0xc2, 0xff, 0xcd, 0xcd, 0xff, 0xc4, 0xc4, 0xff, 0xb9, 0xb9, 0xf2, 0x96, 0x96, 0xb1, 0x50, 0x50, 0x4e, 0x09, 0x09, 0x0d, 0x00, 0x00, 
//	0x12, 0x08, 0x06, 0x00, 0x00, 0x36, 0x36, 0x36, 0x70, 0x54, 0x54, 0x94, 0x6c, 0x6c, 0x95, 0x5d, 0x5d, 0x4e, 0x0c, 0x0c, 0x13, 0x00, 0x00, 0x01, 0x00, 0x00, 
//	0x14, 0x04, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 
//};
//extern __declspec(selectany) POINT const g_tHotSpot = {3, 30};

extern __declspec(selectany) BYTE const g_aPattern[] = // the image is upside down
{
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x20, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x0b, 0x0d, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x07, 0x11, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x06, 0x12, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x06, 0x12, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x05, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x18, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x18, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x08, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x0b, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x0a, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x09, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x08, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x06, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x05, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x04, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x03, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x02, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x01, 0xff, 0x00, 0x00, 
};
extern __declspec(selectany) POINT const g_tHotSpot = {0, 0};


class CPixelColorPicker : public CWindowImpl<CPixelColorPicker>
{
public:
	DECLARE_WND_CLASS_EX(_T("PixelColorPicker"), 0, -1) // no background brush

	// returns true if color was picked
	static bool PickColor(COLORREF* a_pColor)
	{
		CPixelColorPicker wnd;
		wnd.Create(NULL, 0, _T("Pixel Color Picker"), WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, WS_EX_TRANSPARENT|WS_EX_TOPMOST);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) // not entirely correct, but...
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!wnd.m_bValid)
			return false;
		*a_pColor = wnd.m_clrPicked;
		return true;
	}

private:
	// private constructor - do not create instances of this window - just use CPixelColorPicker::PickColor
	CPixelColorPicker() : m_bValid(false), m_hLastCursor(NULL)
	{
	}
	~CPixelColorPicker()
	{
		if (m_hLastCursor) DestroyCursor(m_hLastCursor);
	}


	BEGIN_MSG_MAP(CPixelColorPicker)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnCancel)
		MESSAGE_HANDLER(WM_KEYDOWN, OnCancel)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnCancel)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnCancel)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnPick)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	LRESULT OnCancel(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& /*a_bHandled*/)
	{
		DestroyWindow();
		PostQuitMessage(0); // break the message pump
		return 0;
	}
	LRESULT OnPick(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		m_clrPicked = GetPixel(hDC, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		ReleaseDC(hDC);
		m_bValid = true;
		return OnCancel(0, 0, 0, a_bHandled);
	}
	LRESULT OnSetCursor(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
			GetVersionEx(&tVersion);
		if (tVersion.dwMajorVersion < 5)
		{
			a_bHandled = FALSE;
			return 0; // custom cursors will not be used on old systems
		}

		HDC hDC = GetDC();
		DWORD dwPos = GetMessagePos();
		COLORREF clr = GetPixel(hDC, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));
		ReleaseDC(hDC);
		HCURSOR hCur = CreateDropperCursor(clr);
		SetCursor(hCur);
		DestroyCursor(m_hLastCursor);
		m_hLastCursor = hCur;
		return 0;
	}

	// Creates cursor from currently picked color.
	// The cursor must be destroyed using DestroyCursor.
	// The created cursor includes alpha channel and
	// therefore it is NOT compatible with WinNT4 and Win98.
	static HCURSOR CreateDropperCursor(COLORREF a_clr)
	{
		#pragma pack( push )
		#pragma pack( 2 )
		struct GRPCURSORDIRENTRY
		{
		   BYTE   bWidth;               // Width, in pixels, of the image
		   BYTE   bHeight;              // Height, in pixels, of the image
		   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
		   BYTE   bReserved;            // Reserved
		   WORD   wX;
		   WORD   wY;
		   DWORD  dwBytesInRes;         // how many bytes in this resource?
		   WORD   nID;                  // the ID
		};

		struct GRPCURSORDIR
		{
		   WORD            idReserved;   // Reserved (must be 0)
		   WORD            idType;       // Resource type (1 for icons)
		   WORD            idCount;      // How many images?
		   GRPCURSORDIRENTRY idEntries[1]; // The entries for each image
		};
		#pragma pack( pop )

		BYTE pData[4 + sizeof(BITMAPINFOHEADER) + 32*32*4 + 4*32]; // hot spot, bitmap header, color data, mask data
		pData[0] = static_cast<BYTE>(g_tHotSpot.x&0xff);
		pData[1] = static_cast<BYTE>((g_tHotSpot.x>>8)&0xff);
		pData[2] = static_cast<BYTE>(g_tHotSpot.y&0xff);
		pData[3] = static_cast<BYTE>((g_tHotSpot.y>>8)&0xff);
		BITMAPINFOHEADER* pHeader = reinterpret_cast<BITMAPINFOHEADER*>(pData+4);
		ZeroMemory(pHeader, sizeof *pHeader);
		pHeader->biSize = sizeof *pHeader;
		pHeader->biWidth = 32;
		pHeader->biHeight = 64; // + mask height
		pHeader->biPlanes = 1;
		pHeader->biBitCount = 32;
		pHeader->biClrUsed = 0;
		pHeader->biClrImportant = 0;
		pHeader->biSizeImage = 32*32*4 + 4*32;
		BYTE* pXOR = reinterpret_cast<BYTE*>(pHeader+1);
		ZeroMemory(pXOR, 32*32*4); // clear the pixels
		int const nR = GetRValue(a_clr);
		int const nG = GetGValue(a_clr);
		int const nB = GetBValue(a_clr);
		BYTE const* pSrc = g_aPattern;
		for (int y = 0; y < 32; ++y)
		{
			BYTE const bToSkip = 32-pSrc[0]-pSrc[1];
			pXOR += 4*pSrc[0];
			pSrc += 2;
			for (BYTE x = pSrc[-1]; x > 0; --x)
			{
				if (*pSrc == 0)
				{
					// transparent pixel
					pXOR[0] = pXOR[1] = pXOR[2] = pXOR[3] = 0;
				}
				else
				{
					pXOR[3] = *pSrc;
					if (pSrc[1] == pSrc[2])
					{
						// grey pixel stays grey
						pXOR[0] = pXOR[1] = pXOR[2] = pSrc[1];
					}
					else
					{
						// colorize the pixel
						//ATLASSERT(pSrc[1] > pSrc[2]);
						BYTE bGrey = pSrc[2];
						BYTE bColor = pSrc[1] - pSrc[2];
						pXOR[0] = static_cast<BYTE>((nB*bColor+127)/255 + bGrey);
						pXOR[1] = static_cast<BYTE>((nG*bColor+127)/255 + bGrey);
						pXOR[2] = static_cast<BYTE>((nR*bColor+127)/255 + bGrey);
					}
				}
				pSrc += 3;
				pXOR += 4;
			}
			pXOR += 4*bToSkip;
		}
		BYTE* pAND = pXOR;
		pXOR -= 4*32*32;
		// create AND mask (TODO: optimize - create the mask only once)
		for (int y = 0; y < 32; ++y)
		{
			for (int x1 = 0; x1 < 4; ++x1)
			{
				BYTE bMask = 0;
				for (int x2 = 0; x2 < 8; ++x2)
				{
					if (pXOR[3] == 0)
					{
						bMask |= 0x80 >> x2;
					}
					pXOR += 4;
				}
				*pAND = bMask;
				++pAND;
			}
		}
		return reinterpret_cast<HCURSOR>(CreateIconFromResourceEx(pData, 4 + sizeof(BITMAPINFOHEADER) + 32*32*4 + 4*32, FALSE, 0x00030000, 32, 32, LR_DEFAULTCOLOR));
	}

private:
	bool m_bValid;
	COLORREF m_clrPicked;
	HCURSOR m_hLastCursor;
};