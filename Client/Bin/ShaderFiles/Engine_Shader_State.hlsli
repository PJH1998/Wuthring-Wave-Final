sampler DefaultSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;
};

sampler PointSampler = sampler_state
{
    filter = min_mag_mip_point;
    AddressU = wrap;
    AddressV = wrap;
};

sampler ClampSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = clamp;
    AddressV = clamp;
};

sampler PointClampSampler = sampler_state
{
    filter = min_mag_mip_point;
    AddressU = clamp;
    AddressV = clamp;
};

SamplerComparisonState ShadowSampler
{
    filter = comparison_min_mag_mip_linear;
    ComparisonFunc = LESS_EQUAL;
};
// Rasterize 
RasterizerState RS_Default
{
    FillMode = solid;
    CullMode = back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Wire
{
    FillMode = wireframe;
    CullMode = back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_None
{
    CullMode = none;
};

RasterizerState RS_Cull_Front
{
    FillMode = solid;
    CullMode = front;
    FrontCounterClockwise = false;
};

// Depth
DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DSS_None
{
    DepthEnable = false;
    DepthWriteMask = zero;
};

DepthStencilState DSS_NoneCompare
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = LESS_EQUAL;
};

// Blend
BlendState BS_Default
{
    BlendEnable[0] = false;
};

BlendState BS_Blend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = ONE;
    DestBlend = ONE;
    BlendOp = Add;
};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = Add;
};

BlendState BS_FXBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = SRC_ALPHA;
    DestBlend = DEST_ALPHA;
    BlendOp = Add;
};
