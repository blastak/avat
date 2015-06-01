#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#include "AdvancedAnnotator.h"

#define KEY_PGDN 0x220000
#define KEY_PGUP 0x210000
#define KEY_DEL 0x2E0000
#define KEY_ENTER 0x0D
#define KEY_BKSPC 0x08
#define KEY_ARROW_LEFT 0x250000
#define KEY_ARROW_UP 0x260000
#define KEY_ARROW_RIGHT 0x270000
#define KEY_ARROW_DOWN 0x280000
#define KEY_NUMPAD_2 0x32
#define KEY_NUMPAD_4 0x34
#define KEY_NUMPAD_6 0x36
#define KEY_NUMPAD_8 0x38

int main()
{
	CAdvancedAnnotator objAnno;
	if (!objAnno.LoadDBFile(CAdvancedAnnotator::DB_VIDEO, "../DBs/Annotation_Demo.mp4"))
		return -1;

	objAnno.LoadXML("Annotation_Demo.xml");
	objAnno.ShowObjectTypes();

	Mat imgInput;
	string strWindowName = "Console version";
	namedWindow(strWindowName, CV_WINDOW_AUTOSIZE);

	setMouseCallback(strWindowName, CAdvancedAnnotator::CallbackMouse, &objAnno);
	objAnno.SetDrawMouseLine(true);

	Mat imgDisp;

	while (1)
	{
		objAnno >> imgInput;
		if (imgDisp.empty()) imgDisp = imgInput.clone();
		else imgInput.copyTo(imgDisp);

		objAnno.DrawRects(imgDisp);
		imshow(strWindowName, imgDisp);

		int key = waitKey(1);
		//printf("%x\n",key);
		if (key == 27)
			break;
		else if (key == KEY_PGDN || key == KEY_ENTER)
			objAnno++;
		else if (key == KEY_PGUP || key == KEY_BKSPC)
			objAnno--;
		else if (key == KEY_DEL)
			objAnno.EventKeyDelete();
		else if (key == KEY_ARROW_LEFT)
			objAnno.EventKeyArrow('L');
		else if (key == KEY_ARROW_RIGHT)
			objAnno.EventKeyArrow('R');
		else if (key == KEY_ARROW_UP)
			objAnno.EventKeyArrow('U');
		else if (key == KEY_ARROW_DOWN)
			objAnno.EventKeyArrow('D');
		else if (key == KEY_NUMPAD_2)
			objAnno.EventKeyNumpad(2);
		else if (key == KEY_NUMPAD_4)
			objAnno.EventKeyNumpad(4);
		else if (key == KEY_NUMPAD_6)
			objAnno.EventKeyNumpad(6);
		else if (key == KEY_NUMPAD_8)
			objAnno.EventKeyNumpad(8);
	}

	return 0;
}
