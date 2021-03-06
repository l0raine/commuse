#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"


class CCOThread;
class ThreadTest;
class HelloWorld;




class HelloWorld : public cocos2d::CCLayer
{
	class ThreadTest : public CCOThread
	{
	public:
		virtual void* ThreadWork(void* p)
		{
			string s;
			s = "ThreadTest";
			pSink->setLab(s.c_str());
			return NULL;
		}
		CC_SYNTHESIZE(HelloWorld*,pSink,Sink);
	};

public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommand to return the exactly class pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);

    // implement the "static node()" method manually
    CREATE_FUNC(HelloWorld);

	void setLab(const char* pStr);

	ThreadTest thread_;

	void update(float dt);

	string sTile;
};



#endif // __HELLOWORLD_SCENE_H__ 