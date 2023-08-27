
#pragma once
// when making auto-operations, put them into one of the following categories
// their location in menu depends on the selected category

// operation is part of the application core
extern __declspec(selectany) CATID const CATID_TagCore = {0xafe7e26, 0x105f, 0x417f, {0x95, 0x44, 0xa9, 0x89, 0x3, 0x32, 0xf5, 0x6c}};

// operation displays user interface when run
extern __declspec(selectany) CATID const CATID_TagInteractive = {0x4f87d339, 0xe29c, 0x4687, {0x87, 0x84, 0xf5, 0xf8, 0xf6, 0xdb, 0xd4, 0x86}}; // {4F87D339-E29C-4687-8784-F5F8F6DBD486}

// operation uses other operations and cannot be used separately
extern __declspec(selectany) CATID const CATID_TagMetaOp = {0x5dce1970, 0xd9b9, 0x4bb0, {0xbe, 0x19, 0xa3, 0x0, 0xaa, 0x92, 0xa7, 0x54}}; // {5DCE1970-D9B9-4bb0-BE19-A300AA92A754}

// operation requires access to window states for proper function
extern __declspec(selectany) CATID const CATID_TagWindowState = {0x38804dc4, 0x74d0, 0x42d0, {0xbd, 0x83, 0xe2, 0x2f, 0x94, 0xe2, 0xe8, 0x4b}}; // {38804DC4-74D0-42d0-BD83-E22F94E2E84B}

// image filters recommended when creating layer style
extern __declspec(selectany) CATID const CATID_TagLayerStyle = {0xb5cc4ece, 0xaad4, 0x4335, {0xb6, 0xa, 0xe6, 0xe1, 0x25, 0x3c, 0x6d, 0x18}}; // {B5CC4ECE-AAD4-4335-B60A-E6E1253C6D18}


// processing of the whole image (symmetry, kaleidoscope, swarm)
extern __declspec(selectany) CATID const CATID_TagImageRearragement = {0xa9f1801, 0x16fa, 0x49ea, {0x96, 0xc3, 0xca, 0xc1, 0xe2, 0x6d, 0xdd, 0xca}}; // {0A9F1801-16FA-49ea-96C3-CAC1E26DDDCA}

// processing of the whole image (noise removal, unsharp mask, vignetting)
extern __declspec(selectany) CATID const CATID_TagImagePhotofix = {0xf5a0fa3a, 0x6c8d, 0x4491, {0x9e, 0x44, 0x45, 0x18, 0x54, 0xc7, 0xe7, 0x7b}}; // {F5A0FA3A-6C8D-4491-9E44-451854C7E77B}

// image transformation (rotation, perspective, polar xform)
extern __declspec(selectany) CATID const CATID_TagImageTransformation = {0xc2e92538, 0x65a, 0x4514, {0x8f, 0x65, 0xda, 0x9a, 0xe6, 0x4e, 0xdc, 0x5b}}; // {C2E92538-065A-4514-8F65-DA9AE64EDC5B}

// image color adjustment and manipulation (curves, levels, autocontrast) and (colorize, posterize)
extern __declspec(selectany) CATID const CATID_TagImageColorAdjustment = {0x6ba5efcd, 0xf26e, 0x4428, {0x80, 0xda, 0x1e, 0x78, 0x46, 0xc9, 0x18, 0x0}}; // {6BA5EFCD-F26E-4428-80DA-1E7846C91800}

// image effects preserving shapes, enhancing the style of the shape (fills, bevel, satin)
extern __declspec(selectany) CATID const CATID_TagImageEmbellishment = {0xf828311c, 0x3938, 0x46f4, {0x97, 0x50, 0xf1, 0x68, 0x73, 0x10, 0xd3, 0x73}}; // {F828311C-3938-46f4-9750-F1687310D373}

// image effects modifying shape (outline)
extern __declspec(selectany) CATID const CATID_TagImageShape = {0x2ce23bbb, 0xfb21, 0x485f, {0x80, 0x61, 0x8f, 0xc6, 0x2d, 0x11, 0xfd, 0x2e}}; // {2CE23BBB-FB21-485f-8061-8FC62D11FD2E}

// images generated from the original and placed under the original (outer sghadow, projected shadow)
extern __declspec(selectany) CATID const CATID_TagImageUnderlay = {0xae587b7f, 0x218f, 0x4b15, {0xa3, 0xcc, 0x7f, 0xce, 0x14, 0x35, 0x52, 0xde}}; // {AE587B7F-218F-4b15-A3CC-7FCE143552DE}

// images generated from the original and blended over (or replacing) the original (glow, luce, blur)
extern __declspec(selectany) CATID const CATID_TagImageOverlay = {0xb3f00b7e, 0x4e01, 0x4643, {0x9a, 0x7c, 0x52, 0xaf, 0x49, 0x1a, 0x74, 0xfe}}; // {B3F00B7E-4E01-4643-9A7C-52AF491A74FE}

// images generated from the original and blended over (or replacing) the original (opacity, set background, watermark)
extern __declspec(selectany) CATID const CATID_TagImageFinalizer = {0xfbbca1ad, 0x9a90, 0x4f9a, {0xac, 0x70, 0x5e, 0x7b, 0x6, 0xfe, 0x6, 0x69}}; // {FBBCA1AD-9A90-4f9a-AC70-5E7B06FE0669}


// image adjustment include filters like levels, brightness, curves, etc.
extern __declspec(selectany) CATID const CATID_TagImageAdjust = {0x3fd24edf, 0xf295, 0x4c06, {0x90, 0xeb, 0x44, 0x38, 0x33, 0x67, 0x51, 0xe5}}; // {3FD24EDF-F295-4c06-90EB-4438336751E5}

// layered image operations - any operation affecting multiple layers (like resample or canvas size)
extern __declspec(selectany) CATID const CATID_TagLayeredImage = {0x78218d44, 0x6891, 0x48e2, {0x91, 0x0f, 0xa9, 0x59, 0xd2, 0x6b, 0xf9, 0x5c}}; // {78218D44-6891-48E2-910F-A959D26BF95C}

// operation with image on a file level, like exporting or printing
extern __declspec(selectany) CATID const CATID_TagImageFile = {0xdaf2640d, 0x4b4c, 0x466a, {0xb5, 0xe1, 0xcc, 0xf5, 0xb5, 0x85, 0x13, 0xff}}; // {DAF2640D-4B4C-466A-B5E1-CCF5B58513FF}

// operation with a whole icon
extern __declspec(selectany) CATID const CATID_TagIcon = {0xef986f2c, 0x10cb, 0x433e, {0x9c, 0x22, 0xa5, 0x1a, 0x2a, 0x94, 0x32, 0xa}}; // {EF986F2C-10CB-433E-9C22-A51A2A94320A}

// operation with U3D file
extern __declspec(selectany) CATID const CATID_TagU3D = {0x8d243df7, 0x8e82, 0x4d7b, {0x9f, 0xe8, 0x17, 0x64, 0x79, 0x34, 0xa6, 0x22}}; // {8D243DF7-8E82-4D7B-9FE8-17647934A622}

