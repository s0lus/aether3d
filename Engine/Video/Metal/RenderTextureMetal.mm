#include "RenderTexture.hpp"
#include "GfxDevice.hpp"
#include "System.hpp"

void ae3d::RenderTexture::Create2D( int aWidth, int aHeight, DataType aDataType, TextureWrap aWrap, TextureFilter aFilter, const char* debugName )
{
    if (aWidth <= 0 || aHeight <= 0)
    {
        System::Print( "Render texture has invalid dimension!\n" );
        return;
    }
    
    width = aWidth;
    height = aHeight;
    wrap = aWrap;
    filter = aFilter;
    handle = 1;
    isRenderTexture = true;
    dataType = aDataType;

    MTLPixelFormat format = MTLPixelFormatBGRA8Unorm_sRGB;

    if (dataType == DataType::R32G32)
    {
        format = MTLPixelFormatRG32Float;
    }
    else if (dataType == DataType::Float)
    {
        format = MTLPixelFormatRGBA32Float;
    }

    MTLTextureDescriptor* textureDescriptor =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                       width:width
                                                      height:height
                                                   mipmapped:NO];
    textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    textureDescriptor.storageMode = MTLStorageModePrivate;
    metalTexture = [GfxDevice::GetMetalDevice() newTextureWithDescriptor:textureDescriptor];

    if (metalTexture == nullptr)
    {
        System::Print( "Failed to create a render texture 2D!\n" );
    }
    
    NSString* debugNameStr = [NSString stringWithUTF8String:debugName ];
    metalTexture.label = debugNameStr;

    MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                         width:width
                                                                                        height:height
                                                                                     mipmapped:NO];

    depthDesc.textureType = MTLTextureType2D;
    depthDesc.sampleCount = 1;
    depthDesc.resourceOptions = MTLResourceStorageModePrivate;
    depthDesc.usage = MTLTextureUsageUnknown;

    metalDepthTexture = [GfxDevice::GetMetalDevice() newTextureWithDescriptor:depthDesc];
    metalDepthTexture.label = @"Render Texture depth";
}

void ae3d::RenderTexture::CreateCube( int aDimension, DataType aDataType, TextureWrap aWrap, TextureFilter aFilter, const char* debugName )
{
    isCube = true;
    width = aDimension;
    height = aDimension;
    wrap = aWrap;
    filter = aFilter;
    handle = 1;
    isRenderTexture = true;
    dataType = aDataType;

    MTLPixelFormat format = MTLPixelFormatBGRA8Unorm_sRGB;

    if (dataType == DataType::R32G32)
    {
        format = MTLPixelFormatRG32Float;
    }
    else if (dataType == DataType::Float)
    {
        format = MTLPixelFormatRGBA32Float;
    }

    MTLTextureDescriptor* textureDescriptor =
    [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:format
                                                       size:width
                                                   mipmapped:NO];
    textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    metalTexture = [GfxDevice::GetMetalDevice() newTextureWithDescriptor:textureDescriptor];
    
    if (metalTexture == nullptr)
    {
        System::Print( "Failed to create a render texture Cube!\n" );
    }
    
    NSString* debugNameStr = [NSString stringWithUTF8String:debugName ];
    metalTexture.label = debugNameStr;

    MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                         width:width
                                                                                        height:height
                                                                                     mipmapped:NO];

    depthDesc.textureType = MTLTextureTypeCube;
    depthDesc.sampleCount = 1;
    depthDesc.resourceOptions = MTLResourceStorageModePrivate;
    depthDesc.usage = MTLTextureUsageUnknown;

    metalDepthTexture = [GfxDevice::GetMetalDevice() newTextureWithDescriptor:depthDesc];
    metalDepthTexture.label = @"Render Texture Cube depth";
}
