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
#include "CCScale9Sprite.h"

USING_NS_CC_EXT;


// This is just a class because the Objective-C version was.  A
// JACModalAlert will never be instanciated.
//
// The one other gotcha' is that your selector targets are *not*
// retained, so make sure they stick around until the dialog is
// released.  (Which may be during an autorelease after the dialog
// disappears.)
//

class JACModalAlert
{
public:
    //Simplest version, text + 1 button (default backgrounds)
    static void popupOnLayer(
                             cocos2d::CCLayer *layer,
                             char const * text,
                             char const * buttonText,
                             cocos2d::CCObject *buttonSelectorTarget,
                             cocos2d::SEL_CallFunc buttonSelector);
    
    // Text + 2 buttons (default backgrounds)
    static void popupOnLayer(
                             cocos2d::CCLayer *layer,
                             char const * text,
                             char const * buttonRightText,
                             char const * buttonLeftText,
                             cocos2d::CCObject *buttonRigthSelectorTarget,
                             cocos2d::SEL_CallFunc buttonRigthSelector,
                             cocos2d::CCObject *buttonLeftSelectorTarget,
                             cocos2d::SEL_CallFunc buttonLeftSelector);
    
    // Text + 1 button + customizable containers
    static void popupOnLayer(
                             cocos2d::CCLayer *layer,
                             char const * text,
                             char const * buttonText,
                             CCScale9Sprite* popup,
                             CCScale9Sprite* textContainer,
                             CCScale9Sprite* button,
                             CCScale9Sprite* buttonSelected,
                             cocos2d::CCObject *buttonSelectorTarget,
                             cocos2d::SEL_CallFunc buttonSelector);
    
    // Text + 2 buttons + customizable containers
	static void popupOnLayer(
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
                             cocos2d::SEL_CallFunc buttonLeftSelector);
    
};
