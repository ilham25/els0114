#pragma once

class CXSLSquareUnit
{
public:
	enum PERSONAL_SHOP_STATE
	{
		PSS_NONE	= 0,
		PSS_WAIT,
		PSS_SHOP,
	};

	enum PERSONAL_SHOP_TYPE
	{
		PST_NONE = 0,

		PST_NORMAL,
		PST_PREMIUM,

		PST_END,
	};

public:
	CXSLSquareUnit();
	~CXSLSquareUnit();
};