
#pragma once
// when making auto-operations, put them into one of the following categories
// their location in menu depends on the selected category

// image effects include filters like drop shadow, blur, crustalize, etc.
extern __declspec(selectany) CATID const CATID_AutoImageEffect = {0xf23c6b90, 0x3452, 0x4f7c, {0xb7, 0xc0, 0x76, 0x47, 0xbc, 0x52, 0x7e, 0xb8}}; // {F23C6B90-3452-4f7c-B7C0-7647BC527EB8}

// image generators overwrite previous image (like fractal generators)
extern __declspec(selectany) CATID const CATID_AutoImageGenerator = {0xf6cd5d16, 0x2f05, 0x45d6, {0xa4, 0xf2, 0x90, 0xbc, 0x31, 0x2b, 0xf6, 0xdd}}; // {F6CD5D16-2F05-45d6-A4F2-90BC312BF6DD}

// image adjustment include filters like levels, brightness, curves, etc.
extern __declspec(selectany) CATID const CATID_AutoImageAdjust = {0x102ad2df, 0xaad0, 0x4173, {0x89, 0x95, 0x8a, 0xf2, 0xfb, 0xfd, 0x44, 0xff}}; // {102AD2DF-AAD0-4173-8995-8AF2FBFD44FF}

// layered image operations - any operation affecting multiple layers (like resample or canvas size)
extern __declspec(selectany) CATID const CATID_AutoLayeredImage = {0x78218d44, 0x6891, 0x48e2, {0x91, 0x0f, 0xa9, 0x59, 0xd2, 0x6b, 0xf9, 0x5c}}; // {78218D44-6891-48E2-910F-A959D26BF95C}

// operation with image on a file level, like exporting or printing
extern __declspec(selectany) CATID const CATID_AutoImageFile = {0xdaf2640d, 0x4b4c, 0x466a, {0xb5, 0xe1, 0xcc, 0xf5, 0xb5, 0x85, 0x13, 0xff}}; // {DAF2640D-4B4C-466A-B5E1-CCF5B58513FF}

// operation with a whole icon
extern __declspec(selectany) CATID const CATID_AutoIcon = {0xef986f2c, 0x10cb, 0x433e, {0x9c, 0x22, 0xa5, 0x1a, 0x2a, 0x94, 0x32, 0xa}}; // {EF986F2C-10CB-433E-9C22-A51A2A94320A}

// operation with U3D file
extern __declspec(selectany) CATID const CATID_AutoU3D = {0x8d243df7, 0x8e82, 0x4d7b, {0x9f, 0xe8, 0x17, 0x64, 0x79, 0x34, 0xa6, 0x22}}; // {8D243DF7-8E82-4D7B-9FE8-17647934A622}
