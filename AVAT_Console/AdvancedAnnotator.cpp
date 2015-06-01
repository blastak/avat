#include "AdvancedAnnotator.h"

#define LOG_OUT(fmt) printf( "[%s:%d] %s\n",__FUNCTION__,__LINE__,fmt)
#define RECT_MINIMUM_AREA 100
#define RECT_SELECT_DIST 5

CAdvancedAnnotator::CAdvancedAnnotator(void)
{
	m_idxSelectedRect = -1;
	m_rectDrawing = Rect_<int>(-1,-1,0,0);
	m_bMouseLine = false;
	m_bMouseEvent = false;
	m_mapGT.clear();
	m_vecType.clear();
}

CAdvancedAnnotator::~CAdvancedAnnotator(void)
{
}

bool CAdvancedAnnotator::LoadXML( string strFileName )
{
	m_strXMLFileName = strFileName;
	string strName = m_strPathAbsolute+strFileName;
	LoadXmlDoc( strName );
	EnrollXmlRoot();
	m_pXMLSequence = EnrollXmlChild(m_pXMLRoot, "SequenceInfo");
	m_pXMLObjectType = EnrollXmlChild(m_pXMLRoot, "ObjectTypeInfo");
	m_pXMLFrameInfo = EnrollXmlChild(m_pXMLRoot, "FrameInfo");

	string strType = (m_eDBmode==DB_IMAGE ? "Image":"Video");
	m_pXMLSequence->SetAttribute("SeqType", strType.c_str());

	TiXmlElement* elemImgOrVid = EnrollXmlChild(m_pXMLSequence, strType);
	elemImgOrVid->SetAttribute("FileName", m_objDBSeq->GetFileName().c_str());
	elemImgOrVid->SetAttribute("TotalFrames", m_nTotalFrame);
	elemImgOrVid->SetAttribute("FrameWidth", m_sizeFrame.width);
	elemImgOrVid->SetAttribute("FrameHeight", m_sizeFrame.height);

	ReadOnceXmlObjectTypeInfo();

	ReadOnceXmlFrameInfo();

	GoToSpecificFrame(0);

	return true;
}

void CAdvancedAnnotator::LoadXmlDoc( string strName )
{
	m_pXMLDoc = new TiXmlDocument();
	if(!m_pXMLDoc->LoadFile(strName.c_str()))
	{
		TiXmlDeclaration* pDecl = new TiXmlDeclaration("1.0", "UTF-8", "");
		m_pXMLDoc->LinkEndChild(pDecl);
	}
}

void CAdvancedAnnotator::EnrollXmlRoot()
{
	string str = "AdvancedAnnotation";
	m_pXMLRoot = m_pXMLDoc->FirstChildElement(str.c_str());
	if(!m_pXMLRoot)
	{
		m_pXMLRoot = new TiXmlElement(str.c_str());
		m_pXMLDoc->LinkEndChild(m_pXMLRoot);
	}
}

TiXmlElement* CAdvancedAnnotator::EnrollXmlChild( TiXmlElement* pParent, string strChildName )
{
	TiXmlElement* pChild = pParent->FirstChildElement(strChildName.c_str());
	if(!pChild)
	{
		pChild = new TiXmlElement(strChildName.c_str());
		pParent->LinkEndChild(pChild);
	}
	return pChild;
}

void CAdvancedAnnotator::ReadOnceXmlObjectTypeInfo()
{
	static bool flgOnce = false;
	if(flgOnce == true)
		return;
	flgOnce = true;

	TiXmlElement* elemType = m_pXMLObjectType->FirstChildElement("Type");
	for(elemType; elemType; elemType = elemType->NextSiblingElement())
	{
		CObjectInfo infoLoaded;
		infoLoaded.strType = elemType->Attribute("Name");

		TiXmlElement* elemClass = elemType->FirstChildElement("Class");
		for (elemClass; elemClass; elemClass = elemClass->NextSiblingElement())
		{
			infoLoaded.strClass.push_back(elemClass->Attribute("Name"));

			Range rng;
			rng.start = infoLoaded.strSubClass.size();
			TiXmlElement* elemSubClass = elemClass->FirstChildElement("SubClass");
			for (elemSubClass; elemSubClass; elemSubClass = elemSubClass->NextSiblingElement())
			{
				infoLoaded.strSubClass.push_back(elemSubClass->Attribute("Name"));
			}
			rng.end = infoLoaded.strSubClass.size()-1;

			infoLoaded.rangeSubClass.push_back(rng);
		}

		m_vecType.push_back(infoLoaded);
	}
}

void CAdvancedAnnotator::ReadOnceXmlFrameInfo()
{
	static bool flgOnce = false;
	if(flgOnce == true)
		return;
	flgOnce = true;

	TiXmlElement* elemFrame = m_pXMLFrameInfo->FirstChildElement("Frame");
	CGroundTruth objGT;
	for(elemFrame; elemFrame; elemFrame = elemFrame->NextSiblingElement()) // Frame 찾기
	{
		int nFrame = -1;
		elemFrame->Attribute("Number",&nFrame);
		TiXmlElement* elemObject = elemFrame->FirstChildElement("Object");
		for (elemObject; elemObject; elemObject = elemObject->NextSiblingElement())
		{
			objGT.Clear();

			elemObject->Attribute("Id", &(objGT.Id));
			elemObject->Attribute("Uid", &(objGT.uniqueID));
			objGT.strObjType = elemObject->Attribute("Type");
			objGT.strObjClass = elemObject->Attribute("Class");
			objGT.strObjSubClass = elemObject->Attribute("SubClass");
			elemObject->Attribute("x", &(objGT.rectROI.x));
			elemObject->Attribute("y", &(objGT.rectROI.y));
			elemObject->Attribute("w", &(objGT.rectROI.width));
			elemObject->Attribute("h", &(objGT.rectROI.height));

			objGT.eCondition = CGroundTruth::GT_SAVE;

			m_mapGT[nFrame].push_back(objGT);
		}
	}
}

void CAdvancedAnnotator::ApplyNewObjectToXml(const int knIdx)
{
	const int knFrame = m_nCurrFrame;

	TiXmlElement* elemFrame = m_pXMLFrameInfo->FirstChildElement("Frame");

	bool bFrameExist = false;
	for(elemFrame; elemFrame; elemFrame = elemFrame->NextSiblingElement())
	{
		int nFrameXML = -1;
		elemFrame->Attribute("Number",&nFrameXML);
		if(nFrameXML >= knFrame)
		{
			bFrameExist = (nFrameXML == knFrame); //T:존재, F:지나감
			break;
		}
	}

	int nSetId = 0;
	if (!elemFrame) // 한개도 없을 때
	{
		elemFrame = new TiXmlElement("Frame");
		m_pXMLFrameInfo->LinkEndChild(elemFrame);
		elemFrame->SetAttribute("Number", knFrame);
	}
	else if(!bFrameExist) // 지나갔을 때
	{
		TiXmlElement elemNewFrame("Frame");
		elemNewFrame.SetAttribute("Number", knFrame);
		elemFrame = (TiXmlElement*)(m_pXMLFrameInfo->InsertBeforeChild(elemFrame, elemNewFrame));
	}
	else
	{
		TiXmlElement* tempObject = elemFrame->LastChild("Object")->ToElement();
		if(tempObject) tempObject->Attribute("Id",&nSetId);
		++nSetId;
	}

	m_mapGT[m_nCurrFrame][knIdx].Id = nSetId;

	TiXmlElement* elemObject = new TiXmlElement("Object");
	elemFrame->LinkEndChild(elemObject);
	elemObject->SetAttribute("Id", nSetId);
	elemObject->SetAttribute("Uid", -1);
	elemObject->SetAttribute("Type", m_mapGT[m_nCurrFrame][knIdx].strObjType.c_str());
	elemObject->SetAttribute("Class", m_mapGT[m_nCurrFrame][knIdx].strObjClass.c_str());
	elemObject->SetAttribute("SubClass", m_mapGT[m_nCurrFrame][knIdx].strObjSubClass.c_str());
	elemObject->SetAttribute("x", m_mapGT[m_nCurrFrame][knIdx].rectROI.x);
	elemObject->SetAttribute("y", m_mapGT[m_nCurrFrame][knIdx].rectROI.y);
	elemObject->SetAttribute("w", m_mapGT[m_nCurrFrame][knIdx].rectROI.width);
	elemObject->SetAttribute("h", m_mapGT[m_nCurrFrame][knIdx].rectROI.height);

	m_mapGT[m_nCurrFrame][knIdx].eCondition = CGroundTruth::GT_SAVE;
}

void CAdvancedAnnotator::ApplyModifyObjectToXml(const int knIdx)
{
	const int knFrame = m_nCurrFrame;

	TiXmlElement* elemFrame = m_pXMLFrameInfo->FirstChildElement("Frame");

	for(elemFrame; elemFrame; elemFrame = elemFrame->NextSiblingElement())
	{
		int nFrameTemp = -1;
		elemFrame->Attribute("Number",&nFrameTemp);
		if(nFrameTemp == knFrame)
			break;
	}

	TiXmlElement* elemObject = elemFrame->FirstChildElement("Object");
	for (elemObject; elemObject; elemObject = elemObject->NextSiblingElement())
	{
		int nIdTemp = -1;
		elemObject->Attribute("Id",&nIdTemp);
		if(nIdTemp == m_mapGT[m_nCurrFrame][knIdx].Id)
			break;
	}

	//elemObject->SetAttribute("Id", m_mapGT[nFrame][knIdx].Id); // same
	elemObject->SetAttribute("Uid", -1);
	elemObject->SetAttribute("Type", m_mapGT[m_nCurrFrame][knIdx].strObjType.c_str());
	elemObject->SetAttribute("Class", m_mapGT[m_nCurrFrame][knIdx].strObjClass.c_str());
	elemObject->SetAttribute("SubClass", m_mapGT[m_nCurrFrame][knIdx].strObjSubClass.c_str());
	elemObject->SetAttribute("x", m_mapGT[m_nCurrFrame][knIdx].rectROI.x);
	elemObject->SetAttribute("y", m_mapGT[m_nCurrFrame][knIdx].rectROI.y);
	elemObject->SetAttribute("w", m_mapGT[m_nCurrFrame][knIdx].rectROI.width);
	elemObject->SetAttribute("h", m_mapGT[m_nCurrFrame][knIdx].rectROI.height);

	m_mapGT[m_nCurrFrame][knIdx].eCondition = CGroundTruth::GT_SAVE;
}

void CAdvancedAnnotator::ApplyDeleteObjectToXml(const int knIdx)
{
	const int knFrame = m_nCurrFrame;

	TiXmlElement* elemFrame = m_pXMLFrameInfo->FirstChildElement("Frame");

	int nFrameTemp = -1;
	for(elemFrame; elemFrame; elemFrame = elemFrame->NextSiblingElement())
	{
		elemFrame->Attribute("Number",&nFrameTemp);
		if(nFrameTemp == knFrame)
			break;
	}

	if(nFrameTemp == knFrame)
	{
		TiXmlElement* elemObject = elemFrame->FirstChildElement("Object");
		for (elemObject; elemObject; elemObject = elemObject->NextSiblingElement())
		{
			int nIdTemp = -1;
			elemObject->Attribute("Id",&nIdTemp);
			if(nIdTemp == m_mapGT[m_nCurrFrame][knIdx].Id)
				break;
		}

		elemFrame->RemoveChild(elemObject);
		elemObject = elemFrame->FirstChildElement("Object");
		if(!elemObject)
			m_pXMLFrameInfo->RemoveChild(elemFrame);
	}
	m_mapGT[m_nCurrFrame].erase(m_mapGT[m_nCurrFrame].begin() + knIdx);
}

void CAdvancedAnnotator::SyncXmlFrameInfo()
{
	if (m_flgVecAlloc) // map 에 없고
	{
		if (m_pvecGTSaved->size() > 0) // vec 에 있는 경우
			m_mapGT[m_nCurrFrame] = *m_pvecGTSaved; // 복사

		delete m_pvecGTSaved;
		m_flgVecAlloc = false;
	}
	else
	{
		for (size_t i=0; i<m_mapGT[m_nCurrFrame].size(); i++)
		{
			switch(m_mapGT[m_nCurrFrame][i].eCondition)
			{
			case CGroundTruth::GT_NEW:
				ApplyNewObjectToXml(i);
				break;
			case CGroundTruth::GT_MODIFY:
				ApplyModifyObjectToXml(i);
				break;
			case CGroundTruth::GT_DELETE:
				ApplyDeleteObjectToXml(i);
				--i;
				break;
			}
		}
	}	
}

//@override
bool CAdvancedAnnotator::GoToSpecificFrame( int nSpecFrame )
{
	if(m_nCurrFrame != nSpecFrame)
	{
		SyncXmlFrameInfo();
		m_pXMLDoc->SaveFile((m_strPathAbsolute+m_strXMLFileName).c_str());
	}

	if(nSpecFrame < 0 || nSpecFrame >= m_nTotalFrame)
	{
		char szErr[100];
		sprintf_s(szErr, "frame is out-of-range : it should be 0~%d..\n",m_nTotalFrame-1);
		LOG_OUT(szErr);
		return false;
	}

	LoadFrame(nSpecFrame);

	return true;
}

void CAdvancedAnnotator::LoadFrame(int nSpecFrame)
{
	m_nCurrFrame = nSpecFrame;

	m_idxSelectedRect = -1;
	if(m_mapGT.find(m_nCurrFrame) != m_mapGT.end())
	{
		m_flgVecAlloc = false;
		m_pvecGTSaved = &(m_mapGT[m_nCurrFrame]);
	}
	else
	{
		m_flgVecAlloc = true;
		m_pvecGTSaved = new vector<CGroundTruth>;
	}
}

void CAdvancedAnnotator::CallbackMouse( int myEvent, int x, int y, int keyFlag, void* param )
{
	CAdvancedAnnotator* obj = (CAdvancedAnnotator*)param;

	if(myEvent == EVENT_LBUTTONDOWN)
		obj->EventMouseDown(x,y,keyFlag);
	else if(myEvent == EVENT_MOUSEMOVE)
		obj->EventMouseMove(x,y,keyFlag);
	else if(myEvent == EVENT_LBUTTONUP)
		obj->EventMouseRelease(x,y,keyFlag);
}

void CAdvancedAnnotator::EventMouseDown(int x, int y, int keyFlag)
{
	if(m_bMouseEvent == false)
	{
		m_ptMousePress = Point2i(x,y);
		m_rectDrawing = Rect_<int>(m_ptMousePress,m_ptMousePress);
		m_bMouseEvent = true;
	}
}

void CAdvancedAnnotator::EventMouseMove(int x, int y, int keyFlag)
{
	if(m_bMouseEvent == true)
	{
		m_ptMouseMove = Point2i(x,y);
		m_rectDrawing = Rect_<int>(m_ptMouseMove,m_ptMousePress);
	}

	if(m_bMouseLine)
	{
		m_ptMouseLine[0] = Point2i(0,y); // left
		m_ptMouseLine[1] = Point2i(m_sizeFrame.width,y); // right
		m_ptMouseLine[2] = Point2i(x,0); // top
		m_ptMouseLine[3] = Point2i(x,m_sizeFrame.height); // bottom
	}
}

void CAdvancedAnnotator::EventMouseRelease(int x, int y, int keyFlag)
{
	if(m_bMouseEvent == true)
	{
		m_ptMouseRelease = Point2i(x,y);
		m_rectDrawing = Rect_<int>(m_ptMouseRelease,m_ptMousePress);

		if(m_rectDrawing.area() > RECT_MINIMUM_AREA)
		{
			CGroundTruth tempGT(m_rectDrawing);
			if(m_pvecGTSaved->size() > 0)
				tempGT.Id = m_pvecGTSaved->at(m_pvecGTSaved->size()-1).Id + 1;
			else
				tempGT.Id = 0;
			m_pvecGTSaved->push_back(tempGT);

			m_idxSelectedRect = m_pvecGTSaved->size()-1;
		}
		else
		{
			// 가장 가까운 곳에 위치한 사각형을 선택함
			Rect_<int> rectExpand;
			rectExpand.x = x - RECT_SELECT_DIST;
			rectExpand.y = y - RECT_SELECT_DIST;
			rectExpand.width = RECT_SELECT_DIST * 2;
			rectExpand.height = RECT_SELECT_DIST * 2;

			size_t maxIdx = -1;
			float maxVal = 0;
			for (size_t i=0; i<m_pvecGTSaved->size(); i++)
			{
				int cap = (rectExpand & m_pvecGTSaved->at(i).rectROI).area();
				if(cap == 0) continue;
				else if(m_pvecGTSaved->at(i).eCondition == CGroundTruth::GT_DELETE) continue;

				int cup = (rectExpand | m_pvecGTSaved->at(i).rectROI).area();
				float overlapRatio = (float)cap / (float)cup;
				if(maxVal < overlapRatio)
				{
					maxVal = overlapRatio;
					maxIdx = i;
				}
			}
			m_idxSelectedRect = maxIdx;
		}

		m_rectDrawing = Rect_<int>(-1,-1,0,0);

		m_bMouseEvent = false;
	}
}

void CAdvancedAnnotator::EventKeyDelete()
{
	if(m_idxSelectedRect == -1)
		return;

	m_pvecGTSaved->at(m_idxSelectedRect).eCondition = CGroundTruth::GT_DELETE;
	m_idxSelectedRect = -1;
}

void CAdvancedAnnotator::DrawMouseLine(Mat& img)
{
	line(img, m_ptMouseLine[0], m_ptMouseLine[1], CV_RGB(255,255,255));
	line(img, m_ptMouseLine[2], m_ptMouseLine[3], CV_RGB(255,255,255));
}

void CAdvancedAnnotator::DrawRects(Mat& img)
{
	if(m_bMouseLine)
		DrawMouseLine(img);

	if(m_rectDrawing.area() > 0)
		rectangle(img, m_rectDrawing, CV_RGB(255,255,0));

	for (size_t i=0; i<m_pvecGTSaved->size(); i++)
	{
		if(i == m_idxSelectedRect)
			rectangle(img, m_pvecGTSaved->at(i).rectROI, CV_RGB(0,255,255), 1);
		else if(m_pvecGTSaved->at(i).eCondition != CGroundTruth::GT_DELETE)
			rectangle(img, m_pvecGTSaved->at(i).rectROI, CV_RGB(0,255,0), 2, CV_AA);
	}
}

void CAdvancedAnnotator::EventKeyArrow( char ch )
{
	if(m_idxSelectedRect == -1)
		return;

	switch(ch)
	{
	case 'L':
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.x -= 1;
		break;
	case 'R':
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.x += 1;
		break;
	case 'U':
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.y -= 1;
		break;
	case 'D':
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.y += 1;
		break;
	}
}

void CAdvancedAnnotator::EventKeyNumpad( int num )
{
	if(m_idxSelectedRect == -1)
		return;

	switch(num)
	{
	case 4:
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.width -= 1;
		break;
	case 8:
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.height -= 1;
		break;
	case 6:
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.width += 1;
		break;
	case 2:
		m_pvecGTSaved->at(m_idxSelectedRect).rectROI.height += 1;
		break;
	}
}

bool CAdvancedAnnotator::ShowObjectTypes()
{
	if(m_vecType.size() == 0)
		return false;

	for (size_t i=0; i<m_vecType.size(); i++)
	{
		printf("type%c : %s\n",i+'A',m_vecType[i].strType.c_str());
		for (size_t j=0; j<m_vecType[i].strClass.size(); j++)
		{
			printf("  class%c : %s\n",j+'A',m_vecType[i].strClass[j].c_str());
			for (int k=m_vecType[i].rangeSubClass[j].start, cnt = 0; k<=m_vecType[i].rangeSubClass[j].end; k++)
			{
				printf("    subclass%c : %s\n",'A'+cnt++,m_vecType[i].strSubClass[k].c_str());
			}
		}
	}

	return true;
}

void CAdvancedAnnotator::AddObjectTypes( string strType, string strClass, string strSubClass/*=""*/ )
{
// 	int idxExistType = -1;
// 	int idxExistClass = -1;
// 	int idxExistSubClass = -1;
// 	for (size_t i=0; i<m_vecType.size(); i++)
// 	{
// 		if(m_vecType[i].strType == strType)
// 		{
// 			idxExistType = i;
// 			for (size_t j=0; j<m_vecType[i].strClass.size(); j++)
// 			{
// 				if(m_vecType[i].strClass[j] == strClass)
// 				{
// 					idxExistClass = j;
// 
// 					if(strSubClass.empty())
// 						break;
// 					for (int k=m_vecType[i].rangeSubClass[j].start; k<=m_vecType[i].rangeSubClass[j].end; k++)
// 					{
// 						if(m_vecType[i].strSubClass[k] == strSubClass)
// 						{
// 							idxExistSubClass = k;
// 							break;
// 						}
// 					}
// 					break;
// 				}
// 			}
// 			break;
// 		}
// 	}
// 
// 	if(idxExistType == -1)
// 	{
// 		CObjectInfo infoNew;
// 		infoNew.strType = strType;
// 		infoNew.strClass.push_back(strClass);
// 		if(!strSubClass.empty())
// 		{
// 			infoNew.rangeSubClass.push_back(Range::Range(0,0));
// 			infoNew.strSubClass.push_back(strSubClass);
// 		}
// 		m_vecType.push_back(infoNew);
// 	}
// 	else if(idxExistClass == -1)
// 	{
// 		m_vecType[idxExistType].strClass.push_back(strClass);
// 		if(!strSubClass.empty())
// 		{
// 			m_vecType[idxExistType].rangeSubClass.push_back(Range::Range(0,0));
// 			m_vecType[idxExistType].strSubClass.push_back(strSubClass);
// 		}
// 	}
// 	else if(idxExistSubClass == -1 && !strSubClass.empty())
// 	{
// 		
// 	}
}

void CAdvancedAnnotator::SelectObjectTypes()
{

}
