#include "MultiMap.h"
#include "OGRStyle.h"

DISABLE_WARNINGS
#include "ogr_api.h"
ENABLE_WARNINGS

MULTIMAP_API  OGRStyle::~OGRStyle(void) {
	printf("destructing\n");
}

PROTECTED OGRStyle::OGRStyle(void) {
	unitType = OGRSTUGround;
}

MULTIMAP_API OGRStyle* OGRStyle::Instance(const char* szStyleSpecification ) {
	OGRStyle* ogrStyle = new OGRStyle();
	if ( szStyleSpecification && strlen(szStyleSpecification)>0 ) {
		ogrStyle->styleString = std::string(szStyleSpecification);
		ogrStyle->manager.InitStyleString(szStyleSpecification);
		int partCount = ogrStyle->manager.GetPartCount();
		for ( int p=0; p<partCount; p++ ) {
			OGRStyleTool* pTool = ogrStyle->manager.GetPart(p);
			ogrStyle->styleType = pTool->GetType();
			switch (ogrStyle->styleType) {
			case OGRSTCPen:
				ogrStyle->ParsePen(ogrStyle);
				break;
			case OGRSTCBrush:
				ogrStyle->ParseBrush(ogrStyle);
				break;
			case OGRSTCSymbol:
				ogrStyle->ParseSymbol(ogrStyle);
				break;
			case OGRSTCLabel:
				ogrStyle->ParseLabel(ogrStyle);
				break;
			}
			OGR_ST_Destroy((OGRStyleToolH)pTool);
		}
	}
	return ogrStyle;
}

PRIVATE bool OGRStyle::ParsePen(OGRStyle* pStyle) {
	bool valid = true;
	OGRPen* pPen = (OGRPen*)pStyle;
	int partCount = pStyle->manager.GetPartCount();
	for ( int p=0; p<partCount; p++ ) {
		OGRStylePen* pPenTool = (OGRStylePen*)pStyle->manager.GetPart(p);
		if ( pPenTool ) {
			const char* szStyleString = pPenTool->GetStyleString();
			printf("%s\n",szStyleString);

			GBool isParamNull;
			const char* id = pPenTool->Id(isParamNull);
			if ( !isParamNull ) {
				pPen->id = std::string(id);
			}

			int nRed,nGreen,nBlue,nAlpha;
			const char* colour = pPenTool->Color(isParamNull);
			if ( !isParamNull ) {
				pPenTool->GetRGBFromString(colour,nRed,nGreen,nBlue,nAlpha);
				pPen->color = nRed<<16 | nGreen<<8 | nBlue;
			}

			if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+px") ) {
				pPenTool->SetUnit(OGRSTUPixel);
			} else if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+g") ) {
				pPenTool->SetUnit(OGRSTUGround);
			} else if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+pt") ) {
				pPenTool->SetUnit(OGRSTUPoints);
			} else if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+mm") ) {
				pPenTool->SetUnit(OGRSTUMM);
			} else if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+cm") ) {
				pPenTool->SetUnit(OGRSTUCM);
			} else if ( mfUtils.MatchRegex(szStyleString,"w:[0-9]+in") ) {
				pPenTool->SetUnit(OGRSTUInches);
			}
			double width = pPenTool->Width(isParamNull);
			if ( !isParamNull ) {
				pPen->width = width;
			}

			const char* cap = pPenTool->Cap(isParamNull);
			if ( !isParamNull ) {
				pPen->cap = std::string(cap);
			}

			const char* join = pPenTool->Join(isParamNull);
			if ( !isParamNull ) {
				pPen->join = std::string(join);
			}
			OGR_ST_Destroy(pPenTool);
		}
	}
	return valid;
}
PRIVATE bool OGRStyle::ParseBrush(OGRStyle* pStyle) {
	bool valid = true;

	return valid;
}
PRIVATE bool OGRStyle::ParseSymbol(OGRStyle* pStyle) {
	bool valid = true;

	return valid;
}
PRIVATE bool OGRStyle::ParseLabel(OGRStyle* pStyle) {
	bool valid = true;

	return valid;
}
/////////////////////////////////////////////////////////////////////
MULTIMAP_API OGRPen::~OGRPen(void) {
}
OGRPen::OGRPen(void) {
}
/////////////////////////////////////////////////////////////////////
MULTIMAP_API OGRBrush::~OGRBrush(void) {
}
OGRBrush::OGRBrush(void) {
}
/////////////////////////////////////////////////////////////////////
MULTIMAP_API OGRSymbol::~OGRSymbol(void) {
}
OGRSymbol::OGRSymbol(void) {
}
/////////////////////////////////////////////////////////////////////
MULTIMAP_API OGRLabel::~OGRLabel(void) {
}
OGRLabel::OGRLabel(void) {
}