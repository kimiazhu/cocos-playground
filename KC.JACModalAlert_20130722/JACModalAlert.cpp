/*
 * ModalAlert - Customizable popup dialogs/alerts for Cocos2D-X with a dynamic size
 *
 *
 * Customization of ModalAlert by Rombos && ParallaxShift by
 *
 * Jose Antonio Andujar Clavell, JanduSoft
 * http://www.jandusoft.com
 * http://www.jandujar.com
 *
 * For details, visit the Rombos blog:
 * http://rombosblog.wordpress.com/2012/02/28/modal-alerts-for-cocos2d/
 *
 * Copyright (c) 2012 Hans-Juergen Richstein, Rombos
 * http://www.rombos.de
 *
 * C++ version (c) 2012 Philippe Chaintreuil, ParallaxShift
 * http://www.parallaxshift.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "cocos2d.h"
#include "JACModalAlert.h"
#include "CCControlButton.h"
#include "../utils/HTUtils.h"
#include "../HTConfig.h"

USING_NS_CC;

#define kDialogTag 0xdaa999
#define kCoverLayerTag 0xceaea999
#define kAnimationTime 0.2f
#define kPopuImg "popupBackground.png"
#define kPopupButtonImg "popupButton.png"
//#define kPopupButtonImg "btn_up_108X41.png"
#define kPopupContainerImg "popupContainer.png"
//#define kFont "fonts/futura-48.fnt"
#define kFontName "Helvetica-Bold"
#define kMargingHeight 20
#define kMargingWidth 10
#define kSeparation 20
#define kPopupWidth 400

// Taken from PSModalAlert
// class that implements a black colored layer that will cover the whole screen
// and eats all touches except within the dialog box child
class JACCoverLayer: public CCLayerColor//, public CCTouchDelegate
{
public:
	JACCoverLayer():CCLayerColor() {}
	virtual ~JACCoverLayer() {}

	CREATE_FUNC(JACCoverLayer);

	virtual void onEnter(){
		CCLayerColor::onEnter();
		CCTouchDispatcher* dispatcher = CCDirector::sharedDirector()->getTouchDispatcher();
		dispatcher->addTargetedDelegate(this,INT_MIN+1,true); // swallows touches.
	}
	virtual void onExit() {
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
		CCLayerColor::onExit();
	}

	virtual bool init()
	{
		bool result = false;
		do {
			CC_BREAK_IF(!CCLayerColor::init());

//			this->setTouchEnabled(true);
			result = true;
		} while (0);

		return result;
//		CCSize s = CCDirector::sharedDirector()->getWinSize();
//		return initWithColor(ccc4(0,0,0,255), s.width, s.height);
	}

	virtual bool ccTouchBegan(CCTouch *touch, CCEvent *event)
	{
		return true;
//		CCPoint touchLocation = this->convertTouchToNodeSpace(touch);
//		CCNode *dialogBox = this->getChildByTag(kDialogTag);
//		CC_ASSERT(dialogBox);
//		CCRect const bbox = dialogBox->boundingBox();
//
//		// eat all touches outside of dialog box.
//		return !bbox.containsPoint(touchLocation);
	}
};


// Local function declarations
static void JACModalAlertCloseAlert(
	CCNode *alertDialog,
	CCLayer *coverLayer,
	CCObject *doneSelectorTarget,
	SEL_CallFunc doneSelector);

// Replaces the use of blocks, since C++ doesn't have those.
//
// These would be subclassed from CCObject, but we don't get
// the handy auto-reference counting that blocks get.  So my
// hack is to add them as invisible items on the cover layer.
//
// They can't have circular references back to the layer to run
// the code that was in the blocks, so layers and dialogs are
// found and verified by tags (defines near top).
//
// Like CCMenuItem's, they don't retain the selectors they're
// going to call.
class JACCloseAndCallBlock: public CCNode
{
public:
	JACCloseAndCallBlock():
		selectorTarget(NULL),
		selector(NULL)
	{ }

	virtual ~JACCloseAndCallBlock()
	{
		this->selectorTarget = NULL;
		this->selector = NULL;
	}

	virtual bool initWithOptions(
		CCLayer *coverLayer,
		CCObject *selectorTarget,
		SEL_CallFunc selector)
	{
		// if ( !this->CCNode::init() )
		//	return false;

		CC_ASSERT(coverLayer);
		CC_ASSERT( (selectorTarget && selector) || (!selectorTarget && !selector) );

		// Note that we automatically glom onto the coverlayer,
		// this is likely all that keeps us around.  We'll
		// go away when it goes away.
		CC_ASSERT(coverLayer->getTag() == kCoverLayerTag);
		coverLayer->addChild(this);

		this->setSelectorTarget(selectorTarget);
		this->setSelector(selector);

		return true;
	}

	static JACCloseAndCallBlock* closeAndCallBlockWithOptions(
		CCLayer *coverLayer,
		CCObject *selectorTarget,
		SEL_CallFunc selector)
	{
		JACCloseAndCallBlock *cncb = new JACCloseAndCallBlock();
		if (!cncb)
			return NULL;
		bool success = cncb->initWithOptions(
			coverLayer,
			selectorTarget,
			selector);
		if (success)
			cncb->autorelease();
		else
		{
			delete cncb;
			cncb = NULL;
		}
		return cncb;
	}

	void Execute(CCNode *menu_item)
	{
		CC_UNUSED_PARAM(menu_item);

		// Parent == CoverLayer & find sibling dialog by tag.
		CCNode *parent = this->getParent();
		CC_ASSERT(parent);
		CC_ASSERT(dynamic_cast<CCLayer*>(parent) != NULL);
		CC_ASSERT(parent->getTag() == kCoverLayerTag);

		CCLayer *coverLayer = static_cast<CCLayer*>(parent);
		CC_ASSERT(coverLayer);
		CCNode *dialog = coverLayer->getChildByTag(kDialogTag);
		CC_ASSERT(dialog);

		JACModalAlertCloseAlert(
			dialog,
			coverLayer,
			this->getSelectorTarget(),
			this->getSelector()
		);
	}

	CC_SYNTHESIZE(CCObject*, selectorTarget, SelectorTarget);
	CC_SYNTHESIZE(SEL_CallFunc, selector, Selector);
};

// Make sure you read the comment above JACCloseAndCallBlock
class JACWhenDoneBlock: public CCNode
{
public:
	JACWhenDoneBlock():
		selectorTarget(NULL),
		selector(NULL)
	{ }

	virtual ~JACWhenDoneBlock()
	{
		this->selectorTarget = NULL;
		this->selector = NULL;
	}

	virtual bool initWithOptions(
		CCLayer *coverLayer,
		CCObject *selectorTarget,
		SEL_CallFunc selector)
	{
		// if ( !this->CCNode::init() )
		//	return false;

		CC_ASSERT(coverLayer);
		CC_ASSERT( (selectorTarget && selector) || (!selectorTarget && !selector) );

		// Note that we automatically glom onto the coverlayer,
		// this is likely all that keeps us around.  We'll
		// go away when it goes away.
		CC_ASSERT(coverLayer->getTag() == kCoverLayerTag);
		coverLayer->addChild(this);

		this->setSelectorTarget(selectorTarget);
		this->setSelector(selector);

		return true;
	}

	static JACWhenDoneBlock* whenDone(
		CCLayer *coverLayer,
		CCObject *selectorTarget,
		SEL_CallFunc selector)
	{
		JACWhenDoneBlock *wdb = new JACWhenDoneBlock();
		if (!wdb)
			return NULL;
		bool success = wdb->initWithOptions(
			coverLayer,
			selectorTarget,
			selector);
		if (success)
			wdb->autorelease();
		else
		{
			delete wdb;
			wdb = NULL;
		}
		return wdb;
	}

	void Execute()
	{
		// Code we're replacing:
		//[CCCallBlock actionWithBlock:^{
		//     [coverLayer removeFromParentAndCleanup:YES];
		//     if (block) block();

		// More work than above, coverlayer should be our
		// parent.
		CCNode *parent = this->getParent();
		CC_ASSERT(parent);
		CC_ASSERT(parent->getTag() == kCoverLayerTag);
		CC_ASSERT(dynamic_cast<CCLayer*>(parent) != NULL);
		CCLayer *coverLayer = static_cast<CCLayer*>(parent);
		CC_ASSERT(coverLayer);

		// Retain coverlayer so it doesn't go away on us.
		coverLayer->retain();

		coverLayer->removeFromParentAndCleanup(true);

		SEL_CallFunc sel = this->getSelector();
		if (sel)
		{
			CCObject *target = this->getSelectorTarget();
			CC_ASSERT(target);
			(target->*selector)();
		}
		else
		{
			CC_ASSERT( !this->getSelectorTarget() );
		}

		// Release -- there's a good chance we might be
		// deleted by this next line since being a child
		// of coverLayer is all that keeps us around,
		// so make sure "this" isn't used after it.
		coverLayer->release();
	}

	CC_SYNTHESIZE(CCObject*, selectorTarget, SelectorTarget);
	CC_SYNTHESIZE(SEL_CallFunc, selector, Selector);
};




void JACModalAlertCloseAlert(
	CCNode *alertDialog,
	CCLayer *coverLayer,
	CCObject *doneSelectorTarget,
	SEL_CallFunc doneSelector)
{
	CC_ASSERT(alertDialog);
	CC_ASSERT(coverLayer);

	HTUtils::playSound(SOUND_BUTTON_CLICK);
	// Shrink dialog box
	alertDialog->runAction(
		CCScaleTo::create(kAnimationTime, 0.0f) );

	// in parallel, fadeout and remove cover layer and execute
	// block (note: you can't use CCFadeOut since we don't start at
	// opacity 1!)
	JACWhenDoneBlock *wdb = JACWhenDoneBlock::whenDone(
		coverLayer,
		doneSelectorTarget,
		doneSelector);
	CC_ASSERT(wdb);
//	coverLayer->runAction(CCCallFunc::create(wdb,SEL_CallFunc(&JACWhenDoneBlock::Execute)));
	coverLayer->runAction(
		CCSequence::create(
			CCFadeTo::create(kAnimationTime, 0.0f),
			CCCallFunc::create(wdb,SEL_CallFunc(&JACWhenDoneBlock::Execute)),
			NULL
		));
}

// Text + 2 buttons + customizable containers
void JACModalAlert::popupOnLayer(
                         cocos2d::CCLayer *layer,
                         char const * text,
                         char const * buttonRightText,
                         char const * buttonLeftText,
                         CCScale9Sprite* popup,
                         CCScale9Sprite* textContainer,
                         CCScale9Sprite* buttonRight,
                         CCScale9Sprite* buttonRightSelected,
                         CCScale9Sprite* buttonLeft,
                         CCScale9Sprite* buttonLeftSelected,
                         cocos2d::CCObject *buttonRigthSelectorTarget,
                         cocos2d::SEL_CallFunc buttonRigthSelector,
                         cocos2d::CCObject *buttonLeftSelectorTarget,
                         cocos2d::SEL_CallFunc buttonLeftSelector){
    //Only stricted variables
    CC_ASSERT(layer);
    CC_ASSERT(text);
    CC_ASSERT(buttonRightText);
    CC_ASSERT(popup);
    CC_ASSERT(buttonRight);
    CC_ASSERT(buttonRigthSelectorTarget);
    CC_ASSERT(buttonRigthSelector);

    float maxWidth=0;
    float maxHeight=0;

//    CCLayerColor *maskLayer = JACCoverLayer::create(ccc4(0,0,0,255), 480, 800);

    // Create the cover layer that "hides" the current application.
    JACCoverLayer *coverLayer = JACCoverLayer::create();//node();
	CC_ASSERT(coverLayer);

	// Tag for later validation.
	coverLayer->setTag(kCoverLayerTag);

	// put to the very top to block application touches.
	layer->addChild(coverLayer, INT_MAX);

	// Smooth fade-in to dim with semi-transparency.
	coverLayer->runAction(CCFadeTo::create(kAnimationTime, 220));

	//Create the inner objects

    //Create the text with font
//    CCLabelBMFont *labelText2 = CCLabelBMFont::create(text ,"");
    CCLabelTTF* labelText = CCLabelTTF::create(text, kFontName, 24);
    labelText->setDimensions(CCSizeMake(350, 60));
    labelText->setVerticalAlignment(kCCVerticalTextAlignmentCenter);
    labelText->setHorizontalAlignment(kCCTextAlignmentCenter);
    labelText->setColor(ccc3(0,0,0));
    maxWidth = labelText->getContentSize().width;
    maxHeight = labelText->getContentSize().height;

	//Resize the container for the text
	if(textContainer != NULL){
		maxWidth+=2*kMargingWidth;
		maxHeight+=1.5*kMargingHeight;
		textContainer->setContentSize(CCSizeMake(kPopupWidth-30,maxHeight));
		textContainer->addChild(labelText);

		//Add the label to the container
		CCSize const & tsz = textContainer->getContentSize();
		CCPoint post(tsz.width, tsz.height);
		labelText->setPosition( ccpMult(post, 0.5f));
    }
	//Create the first button
//	CCLabelBMFont *button1Label = CCLabelBMFont ::create(buttonRightText ,kFont);
	CCLabelTTF* button1Label = CCLabelTTF::create(buttonRightText, kFontName, 28);
	button1Label->setColor(ccc3(0xff,0xff,0xff));
	CCControlButton* button1 = CCControlButton::create(button1Label,buttonRight);
//	CCScale9Sprite* sp = CCScale9Sprite::createWithSpriteFrameName("btn_up_126X48.png");
//	CCControlButton* button1 = CCControlButton::create(button1Label,sp);
	button1->setPreferredSize(CCSizeMake(120,50));
    CC_ASSERT(button1);

    //Set the action for the first button
    // The following is to replace the Objective-C block
	// in the original.
	JACCloseAndCallBlock *cncb = JACCloseAndCallBlock::
			closeAndCallBlockWithOptions(
					coverLayer,
					buttonRigthSelectorTarget,
					buttonRigthSelector);
	CC_ASSERT(cncb);

//    button1->addTargetWithActionForControlEvents(cncb, SEL_MenuHandler(&JACCloseAndCallBlock::Execute), CCControlEventTouchDown);
    button1->addTargetWithActionForControlEvents(cncb, SEL_CCControlHandler(&JACCloseAndCallBlock::Execute), CCControlEventTouchUpInside);
	button1->setZoomOnTouchDown(true);
	button1->setTouchPriority(INT_MIN);

	if(buttonRightSelected != NULL){
		button1->setBackgroundSpriteForState(buttonRightSelected, CCControlStateHighlighted);
	}

	if( ( button1->getContentSize().width) > maxWidth){
		maxWidth = button1->getContentSize().width;
	}

	maxHeight+=kSeparation+button1->getContentSize().height;
	float buttonHeight = button1->getContentSize().height;

	CCControlButton* button2=NULL;

	//Create the second button (optional)
	if(buttonLeft!=NULL){
//		CCLabelBMFont *button2Label = CCLabelBMFont ::create(buttonLeftText ,kFont);
		CCLabelTTF* button2Label = CCLabelTTF::create(buttonLeftText, kFontName, 28);
		button2Label->setColor(ccc3(0xff,0xff,0xff));
		button2 = CCControlButton::create(button2Label,buttonLeft);
		button2->setPreferredSize(CCSizeMake(120,50));
        CC_ASSERT(button2);

        // Replaces Objective-C block in original code.
		cncb = JACCloseAndCallBlock::closeAndCallBlockWithOptions(
                                                                  coverLayer,
                                                                  buttonLeftSelectorTarget,
                                                                  buttonLeftSelector);
		CC_ASSERT(cncb);
//        button2->addTargetWithActionForControlEvents(cncb, SEL_MenuHandler(&JACCloseAndCallBlock::Execute), CCControlEventTouchDown);
        button2->addTargetWithActionForControlEvents(cncb, SEL_CCControlHandler(&JACCloseAndCallBlock::Execute), CCControlEventTouchUpInside);
		button2->setZoomOnTouchDown(true);
		button2->setTouchPriority(INT_MIN);

		if(buttonLeftSelected != NULL){
			button2->setBackgroundSpriteForState(buttonLeftSelected, CCControlStateHighlighted);
		}

		if( ( button1->getContentSize().width +button2->getContentSize().width + kSeparation ) > maxWidth){
			maxWidth = button1->getContentSize().width +button2->getContentSize().width + kSeparation;
		}

		// Height of button1 must be equal to button2 or we must change the maxHeight
		if( button2->getContentSize().height > button1->getContentSize().height){
			maxHeight-= button1->getContentSize().height;
			maxHeight+= button2->getContentSize().height;
			buttonHeight = button2->getContentSize().height;
		}
	}

	maxWidth+=2*kMargingWidth;
	maxHeight+=2*kMargingHeight;

	//Create the popup dialog
//	popup->setContentSize(CCSizeMake(maxWidth,maxHeight));
	popup->setContentSize(CCSizeMake(kPopupWidth,maxHeight+2*kMargingHeight));

	//add components to popup
    CCSize const & sz = coverLayer->getContentSize();
	CCPoint pos(sz.width, sz.height);

	//add the text
	if(textContainer != NULL){
		popup->addChild(textContainer);
//        textContainer->setPosition(ccp(maxWidth*0.5f, maxHeight - kMargingHeight - (textContainer->getContentSize().height/2)));
		textContainer->setPosition(ccp(kPopupWidth*0.5f, maxHeight-/*kMargingHeight-*/(textContainer->getContentSize().height/2)));
	}else{
		popup->addChild(labelText);
        labelText->setPosition(ccp(maxWidth*0.5f, maxHeight-/*kMargingWidth-*/(labelText->getContentSize().height/2)));
	}

	//add buttons
	if(button2!=NULL){
		//Two buttons
		popup->addChild(button1);
		popup->addChild(button2);
//        button1->setPosition(ccp((button1->getContentSize().width/2) + kMargingWidth, kMargingHeight + (button1->getContentSize().height/2)));
//        button2->setPosition(ccp(maxWidth -(button2->getContentSize().width/2) - kMargingWidth , kMargingHeight + (button2->getContentSize().height/2)));
        button1->setPosition(ccp(kPopupWidth/2-button1->getContentSize().width, 2*kMargingHeight + (button1->getContentSize().height/2)));
		button2->setPosition(ccp(kPopupWidth/2+button1->getContentSize().width, 2*kMargingHeight + (button2->getContentSize().height/2)));
	}else{
		//One Button
		popup->addChild(button1);
        button1->setPosition(ccp(kPopupWidth*0.5f, 2*kMargingHeight + (button1->getContentSize().height/2)));
	}


	//Add the popup to the cover layer
	popup->setTag(kDialogTag);
    popup->setPosition( ccpMult(pos, 0.5f) );
    //TODO: setOpacity doesn't work on children
//	popup->setOpacity(220); // Make it a bit transparent for a cooler look.

	coverLayer->addChild(popup);

	// Open the dialog with a nice popup-effect
	popup->setScale(0.0f);
	popup->runAction(
		CCEaseBackOut::create(
			CCScaleTo::create(
				kAnimationTime,
				1.0f)
		) );
}


// Text + 1 button + customizable colors
void JACModalAlert::popupOnLayer(
                         cocos2d::CCLayer *layer,
                         char const * text,
                         char const * buttonText,
                         CCScale9Sprite* popup,
                         CCScale9Sprite* textContainer,
                         CCScale9Sprite* button,
                         CCScale9Sprite* buttonSelected,
                         cocos2d::CCObject *buttonSelectorTarget,
                         cocos2d::SEL_CallFunc buttonSelector){
    CC_ASSERT(layer);
    CC_ASSERT(text);
    CC_ASSERT(buttonText);
    CC_ASSERT(popup);
    CC_ASSERT(button);
    CC_ASSERT(buttonSelectorTarget);
    CC_ASSERT(buttonSelector);

    popupOnLayer(layer, text, buttonText, NULL, popup, textContainer, button, NULL, NULL, NULL, buttonSelectorTarget, buttonSelector, NULL, NULL);

}

// Text + 2 buttons (default colors)
void JACModalAlert::popupOnLayer(
                         cocos2d::CCLayer *layer,
                         char const * text,
                         char const * buttonRightText,
                         char const * buttonLeftText,
                         cocos2d::CCObject *buttonRigthSelectorTarget,
                         cocos2d::SEL_CallFunc buttonRigthSelector,
                         cocos2d::CCObject *buttonLeftSelectorTarget,
                         cocos2d::SEL_CallFunc buttonLeftSelector){
    CC_ASSERT(layer);
    CC_ASSERT(text);
    CC_ASSERT(buttonRightText);
    CC_ASSERT(buttonLeftText);
    CC_ASSERT(buttonRigthSelectorTarget);
    CC_ASSERT(buttonRigthSelector);
    CC_ASSERT(buttonLeftSelectorTarget);
    CC_ASSERT(buttonLeftSelector);

    CCScale9Sprite *popup = CCScale9Sprite::createWithSpriteFrameName(kPopuImg);
    CCScale9Sprite *textContainer = CCScale9Sprite::createWithSpriteFrameName(kPopupContainerImg);
    CCScale9Sprite *button = CCScale9Sprite::createWithSpriteFrameName(kPopupButtonImg);
    CCScale9Sprite *button2 = CCScale9Sprite::createWithSpriteFrameName(kPopupButtonImg);

    popupOnLayer(layer, text, buttonRightText, buttonLeftText, popup, textContainer, button, NULL, button2, NULL, buttonRigthSelectorTarget, buttonRigthSelector, buttonLeftSelectorTarget, buttonLeftSelector);

}

// Text + 1 button (default colors)
void JACModalAlert::popupOnLayer(
                         cocos2d::CCLayer *layer,
                         char const * text,
                         char const * buttonText,
                         cocos2d::CCObject *buttonSelectorTarget,
                         cocos2d::SEL_CallFunc buttonSelector){

	CC_ASSERT(layer);
    CC_ASSERT(text);
    CC_ASSERT(buttonText);
    CC_ASSERT(buttonSelectorTarget);
    CC_ASSERT(buttonSelector);
    
    CCScale9Sprite *popup = CCScale9Sprite::createWithSpriteFrameName(kPopuImg);
    CCScale9Sprite *textContainer = CCScale9Sprite::createWithSpriteFrameName(kPopupContainerImg);
    CCScale9Sprite *button = CCScale9Sprite::createWithSpriteFrameName(kPopupButtonImg);
    
    popupOnLayer(layer, text, buttonText, NULL, popup, textContainer, button, NULL, NULL, NULL, buttonSelectorTarget, buttonSelector, NULL, NULL);


}
