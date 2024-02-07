#include "CoreMinimalObject.h"

vector<CCoreMinimalObject*> GObjects;

CCoreMinimalObject::CCoreMinimalObject()
{
	bTick = true;
	GObjects.push_back(this);
}

CCoreMinimalObject::~CCoreMinimalObject()
{
	for (vector<CCoreMinimalObject*>::const_iterator iter = GObjects.begin();
		iter != GObjects.end();
		++iter)
	{
		if (*iter == this)
		{
			GObjects.erase(iter);
			break;
		}
	}
}
