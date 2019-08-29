/*
Copyright (C) 2014 sapier
Copyright (C) 2014-2019 Maksim Gamarnik [MoNTE48] MoNTE48@mail.ua

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "touchscreengui.h"
#include "irrlichttypes.h"
#include "irr_v2d.h"
#include "log.h"
#include "keycode.h"
#include "settings.h"
#include "gettime.h"
#include "util/numeric.h"
#include "porting.h"
#include "guiscalingfilter.h"

#include <iostream>
#include <algorithm>

#include <ISceneCollisionManager.h>

using namespace irr::core;

const char **touchgui_button_imagenames = (const char *[]) {
	"up_one_btn.png",
	"up_two_btn.png",
	"up_three_btn.png",
	"down_one_btn.png",
	"down_two_btn.png",
	"down_three_btn.png",
	"left_btn.png",
	"right_btn.png",
	"inventory_btn.png",
	"drop_btn.png",
	"jump_btn.png",
	"down_btn.png",
//	"noclip_btn.png",
	"escape_btn.png",
	"minimap_btn.png",
	"rangeview_btn.png",
	"chat_btn.png",
//	"debug_btn.png",
//	"camera_btn.png",
	"empty_btn.png"
};

static irr::EKEY_CODE id2keycode(touch_gui_button_id id) {
	std::string key = key;
	switch (id) {
		case forward_one:
			key = "forward";
			break;
		case forward_two:
			key = "forward";
			break;
		case forward_three:
			key = "forward";
			break;
		case backward_one:
			key = "backward";
			break;
		case backward_two:
			key = "backward";
			break;
		case backward_three:
			key = "backward";
			break;
		case left_id:
			key = "left";
			break;
		case right_id:
			key = "right";
			break;
		case inventory_id:
			key = "inventory";
			break;
		case drop_id:
			key = "drop";
			break;
		case jump_id:
			key = "jump";
			break;
		case crunch_id:
			key = "sneak";
			break;
/*		case noclip_id:
			key = "noclip";
			break;
		case fast_id:
			key = "fast";
			break;
		case debug_id:
			key = "toggle_debug";
			break;*/
		case escape_id:
			return irr::KEY_ESCAPE;
		case minimap_id:
			key = "minimap";
			break;
		case range_id:
			key = "rangeselect";
			break;
		case chat_id:
			key = "chat";
			break;
/*		case camera_id:
			key = "camera_mode";
			break;*/
		case empty_id:
			key = "forward";
			break;
		case after_last_element_id:
			break;
	}
	assert(!key.empty());
	return keyname_to_keycode(g_settings->get("keymap_" + key).c_str());
}

TouchScreenGUI *g_touchscreengui;

static void load_button_texture(button_info *btn, const char *path,
								rect<s32> button_rect, ISimpleTextureSource *tsrc,
								video::IVideoDriver *driver) {
	unsigned int tid;
	video::ITexture *texture = guiScalingImageButton(driver,
														tsrc->getTexture(path, &tid),
														button_rect.getWidth(),
														button_rect.getHeight());
	if (texture) {
		btn->guibutton->setUseAlphaChannel(true);
		if (g_settings->getBool("gui_scaling_filter")) {
			rect<s32> txr_rect = rect<s32>(0, 0, button_rect.getWidth(), button_rect.getHeight());
			btn->guibutton->setImage(texture, txr_rect);
			btn->guibutton->setPressedImage(texture, txr_rect);
			btn->guibutton->setScaleImage(false);
		} else {
			btn->guibutton->setImage(texture);
			btn->guibutton->setPressedImage(texture);
			btn->guibutton->setScaleImage(true);
		}
		btn->guibutton->setDrawBorder(false);
		btn->guibutton->setText(L"");
	}
}

TouchScreenGUI::TouchScreenGUI(IrrlichtDevice *device, IEventReceiver *receiver) :
	m_device(device),
	m_guienv(device->getGUIEnvironment()),
	m_camera_yaw_change(0.0),
	m_camera_pitch(0.0),
	m_visible(false),
	m_move_id(-1),
	m_receiver(receiver),
	m_move_has_really_moved(false),
	m_move_downtime(0),
	m_move_sent_as_mouse_event(false),
	// use some downlocation way off screen as init value to avoid invalid behaviour
	m_move_downlocation(v2s32(-10000, -10000)) {
		for (unsigned int i = 0; i < after_last_element_id; i++) {
		m_buttons[i].guibutton     =  0;
		m_buttons[i].repeatcounter = -1;
		m_buttons[i].repeatdelay   = BUTTON_REPEAT_DELAY;
	}

	m_screensize = m_device->getVideoDriver()->getScreenSize();
}

void TouchScreenGUI::initButton(touch_gui_button_id id, rect<s32> button_rect,
								std::wstring caption, bool immediate_release, float repeat_delay) {

	button_info *btn       = &m_buttons[id];
	btn->guibutton         = m_guienv->addButton(button_rect, nullptr, id, caption.c_str());
	btn->guibutton->grab();
	btn->repeatcounter     = -1;
	btn->repeatdelay       = repeat_delay;
	btn->keycode           = id2keycode(id);
	btn->immediate_release = immediate_release;
	btn->ids.clear();

	load_button_texture(btn, touchgui_button_imagenames[id], button_rect,
						m_texturesource, m_device->getVideoDriver());
}

void TouchScreenGUI::init(ISimpleTextureSource *tsrc) {
	assert(tsrc != nullptr);

	float control_pad_size =
			MYMIN(m_screensize.Y / 1.5,
					porting::getDisplayDensity() * g_settings->getFloat("hud_scaling") * 260);

	u32 button_size      = control_pad_size / 3;
	m_visible            = true;
	m_texturesource      = tsrc;
	m_control_pad_rect   = rect<s32>(0, m_screensize.Y - control_pad_size,
										control_pad_size, m_screensize.Y);

	/*
	draw control pad
	0 3 6
	1 4 7
	2 5 8
	*/

	int number = 0;
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x, ++number) {
			v2s32 tl;
			tl.X = button_size * y;
			tl.Y = m_screensize.Y - button_size * (3 - x);

			rect<s32> button_rect(tl.X, tl.Y, tl.X + button_size, tl.Y + button_size);
			touch_gui_button_id id = after_last_element_id;
			std::wstring caption;
			switch (number) {
				case 0:
					id = forward_one;
					caption = L"^";
					break;
				case 3:
					id = forward_two;
					caption = L"^";
					break;
				case 6:
					id = forward_three;
					caption = L"^";
					break;
				case 1:
					id = left_id;
					caption = L"<";
					break;
				case 4:
					id = empty_id;
					break;
				case 2:
					id = backward_one;
					caption = L"v";
					break;
				case 5:
					id = backward_two;
					caption = L"v";
					break;
				case 8:
					id = backward_three;
					caption = L"v";
					break;
				case 7:
					id = right_id;
					caption = L">";
					break;
			}
			if (id != after_last_element_id) {
				initButton(id, button_rect, caption, false);
			}
		}

	// init inventory button
	initButton(inventory_id,
			rect<s32>(m_screensize.X - (button_size),
					  m_screensize.Y - (button_size),
					  m_screensize.X,
					  m_screensize.Y),
			L"inv", false, SLOW_BUTTON_REPEAT);

	// init drop button
	initButton(drop_id,
			rect<s32>(m_screensize.X - (button_size * 0.75),
					  m_screensize.Y / 2 - (button_size * 1.5),
					  m_screensize.X,
					  m_screensize.Y / 2 - (0.75 * button_size)),
			L"drop", false, SLOW_BUTTON_REPEAT);

	// init crunch button
	initButton(crunch_id,
			rect<s32>(m_screensize.X - (button_size * 2),
					  m_screensize.Y - (button_size * 0.5),
					  m_screensize.X - (button_size),
					  m_screensize.Y),
			L"H", false, SLOW_BUTTON_REPEAT);

	// init jump button
	initButton(jump_id,
			rect<s32>(m_screensize.X - (button_size * 2),
					  m_screensize.Y - (button_size * 2),
					  m_screensize.X - (button_size),
					  m_screensize.Y - (button_size)),
			L"x", false, SLOW_BUTTON_REPEAT);

/*
	// init noclip button
	initButton(noclip_id,
			rect<s32>(m_screensize.X - (button_size * 0.75),
					  m_screensize.Y - (button_size * 4.75),
					  m_screensize.X,
					  m_screensize.Y - (button_size * 4)),
			L"clip", false, SLOW_BUTTON_REPEAT);
	// init fast button
	initButton(fast_id,
			rect<s32>(m_screensize.X - (button_size * 0.75),
					  m_screensize.Y - (button_size * 4),
					  m_screensize.X,
					  m_screensize.Y - (button_size * 3.25)),
			L"fast", false, SLOW_BUTTON_REPEAT);
*/

// iOS does not have a physical pause button and have memory leak with minimap
#ifdef __IOS__
	// init pause button
	initButton(escape_id,
		rect<s32>(m_screensize.X / 2 - (button_size * 1.125), 0,
					  m_screensize.X / 2 - (button_size * 0.375),
					 (button_size * 0.75)),
		L"Exit", false, SLOW_BUTTON_REPEAT);

#else
	// init minimap button
	initButton(minimap_id,
			rect<s32>(m_screensize.X / 2 - (button_size * 1.125), 0,
					  m_screensize.X / 2 - (button_size * 0.375),
					 (button_size * 0.75)),
			L"minimap", false, SLOW_BUTTON_REPEAT);
#endif
	// init rangeselect button
	initButton(range_id,
			rect<s32>(m_screensize.X / 2 - (button_size * 0.375), 0,
					  m_screensize.X / 2 + (button_size * 0.375),
					 (button_size * 0.75)),
			L"far", false, SLOW_BUTTON_REPEAT);

	// init chat button
	initButton(chat_id,
			rect<s32>(m_screensize.X / 2 + (button_size * 0.375), 0,
					  m_screensize.X / 2 + (button_size * 1.125),
					 (button_size * 0.75)),
			L"Chat", false, SLOW_BUTTON_REPEAT);

	// init camera button
/*	initButton(camera_id,
			rect<s32>(0, 0,
					button_size * 0.75, button_size * 0.75),
			L"cam", false, SLOW_BUTTON_REPEAT);*/
}

touch_gui_button_id TouchScreenGUI::getButtonID(s32 x, s32 y) {
	IGUIElement *rootguielement = m_guienv->getRootGUIElement();

	if (rootguielement != nullptr) {
		gui::IGUIElement *element =
				rootguielement->getElementFromPoint(core::position2d<s32>(x, y));

		if (element) {
			for (unsigned int i = 0; i < after_last_element_id; i++) {
				if (element == m_buttons[i].guibutton) {
					return (touch_gui_button_id) i;
				}
			}
		}
	}
	return after_last_element_id;
}

touch_gui_button_id TouchScreenGUI::getButtonID(size_t eventID) {
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		button_info *btn = &m_buttons[i];

		std::vector<size_t>::iterator id =
				std::find(btn->ids.begin(), btn->ids.end(), eventID);

		if (id != btn->ids.end())
			return (touch_gui_button_id) i;
	}

	return after_last_element_id;
}

bool TouchScreenGUI::isHUDButton(const SEvent &event) {
	// check if hud item is pressed
	for (std::map<int,rect<s32> >::iterator iter = m_hud_rects.begin();
			iter != m_hud_rects.end(); ++iter) {
		if (iter->second.isPointInside(
				v2s32(event.TouchInput.X,
					  event.TouchInput.Y)
		)) {
			if (iter->first < 9) {
				SEvent* translated = new SEvent();
				memset(translated, 0, sizeof(SEvent));
				translated->EventType = irr::EET_KEY_INPUT_EVENT;
				translated->KeyInput.Key         = (irr::EKEY_CODE) (KEY_KEY_1 + iter->first);
				translated->KeyInput.Control     = false;
				translated->KeyInput.Shift       = false;
				translated->KeyInput.PressedDown = true;
				m_receiver->OnEvent(*translated);
				m_hud_ids[event.TouchInput.ID]   = translated->KeyInput.Key;
				delete translated;
				return true;
			}
		}
	}
	return false;
}

void TouchScreenGUI::handleButtonEvent(touch_gui_button_id button,
										size_t eventID, bool action) {
	button_info *btn = &m_buttons[button];
	SEvent* translated = new SEvent();
	memset(translated, 0, sizeof(SEvent));
	translated->EventType            = irr::EET_KEY_INPUT_EVENT;
	translated->KeyInput.Key         = btn->keycode;
	translated->KeyInput.Control     = false;
	translated->KeyInput.Shift       = false;
	translated->KeyInput.Char        = 0;

	// add this event
	if (action) {
		assert(std::find(btn->ids.begin(), btn->ids.end(), eventID) == btn->ids.end());

		btn->ids.push_back(eventID);

		if (btn->ids.size() > 1)
			return;

		btn->repeatcounter = 0;
		translated->KeyInput.PressedDown = true;
		translated->KeyInput.Key = btn->keycode;
		m_receiver->OnEvent(*translated);
	}
	// remove event
	if ((!action) || (btn->immediate_release)) {

		std::vector<size_t>::iterator pos =
				std::find(btn->ids.begin(),btn->ids.end(), eventID);
		// has to be in touch list
		assert(pos != btn->ids.end());
		btn->ids.erase(pos);

		if (!btn->ids.empty())
			return;

		translated->KeyInput.PressedDown = false;
		btn->repeatcounter               = -1;
		m_receiver->OnEvent(*translated);
	}
	delete translated;
}


void TouchScreenGUI::handleReleaseEvent(size_t evt_id) {
	touch_gui_button_id button = getButtonID(evt_id);

	// handle button events
	if (button != after_last_element_id) {
		handleButtonEvent(button, evt_id, false);
	}
	// handle the point used for moving view
	else if (evt_id == m_move_id) {
		m_move_id = -1;

		// if this pointer issued a mouse event issue symmetric release here
		if (m_move_sent_as_mouse_event) {
			SEvent* translated = new SEvent;
			memset(translated, 0, sizeof(SEvent));
			translated->EventType               = EET_MOUSE_INPUT_EVENT;
			translated->MouseInput.X            = m_move_downlocation.X;
			translated->MouseInput.Y            = m_move_downlocation.Y;
			translated->MouseInput.Shift        = false;
			translated->MouseInput.Control      = false;
			translated->MouseInput.ButtonStates = 0;
			translated->MouseInput.Event        = EMIE_LMOUSE_LEFT_UP;
			m_receiver->OnEvent(*translated);
			delete translated;
		} else if (!m_move_has_really_moved) {
			SEvent* translated = new SEvent;
			memset(translated, 0, sizeof(SEvent));
			translated->EventType               = EET_MOUSE_INPUT_EVENT;
			translated->MouseInput.X            = m_move_downlocation.X;
			translated->MouseInput.Y            = m_move_downlocation.Y;
			translated->MouseInput.Shift        = false;
			translated->MouseInput.Control      = false;
			translated->MouseInput.ButtonStates = 0;
			translated->MouseInput.Event        = EMIE_LMOUSE_LEFT_UP;
			m_receiver->OnEvent(*translated);
			delete translated;
			quickTapDetection();
			m_shootline = m_device
						->getSceneManager()
						->getSceneCollisionManager()
						->getRayFromScreenCoordinates(
							v2s32(m_move_downlocation.X, m_move_downlocation.Y));
		}
	}

	for (std::vector<id_status>::iterator iter = m_known_ids.begin();
			iter != m_known_ids.end(); ++iter) {
		if (iter->id == evt_id) {
			m_known_ids.erase(iter);
			break;
		}
	}
}

#ifdef __IOS__
void TouchScreenGUI::handleReleaseAll()
{
	m_known_ids.clear();
	if(m_move_id != -1)
		handleReleaseEvent(m_move_id);
	for(int i = 0; i < after_last_element_id; i++)
		m_buttons[i].ids.clear();
}
#endif

void TouchScreenGUI::translateEvent(const SEvent &event) {
	if (!m_visible) {
		infostream << "TouchScreenGUI::translateEvent got event but not visible?!" << std::endl;
		return;
	}

	if (event.EventType != EET_TOUCH_INPUT_EVENT)
		return;

	if (event.TouchInput.Event == ETIE_PRESSED_DOWN) {

		/* add to own copy of eventlist ...
		 * android would provide this information but irrlicht guys don't
		 * wanna design a efficient interface */
		id_status toadd{};
		toadd.id = event.TouchInput.ID;
		toadd.X  = event.TouchInput.X;
		toadd.Y  = event.TouchInput.Y;
		m_known_ids.push_back(toadd);

		size_t eventID = event.TouchInput.ID;

		touch_gui_button_id button =
				getButtonID(event.TouchInput.X, event.TouchInput.Y);

		// handle button events
		if (button != after_last_element_id) {
			handleButtonEvent(button, eventID, true);
		} else if (m_control_pad_rect.isPointInside(v2s32(toadd.X, toadd.Y)) || (isHUDButton(event))) {
			// ignore events inside the control pad not already handled
		} else {
			// handle non button events
			// if we don't already have a moving point make this the moving one
			if (m_move_id == -1) {
				m_move_id                  = event.TouchInput.ID;
				m_move_has_really_moved    = false;
				m_move_downtime            = porting::getTimeMs();
				m_move_downlocation        = v2s32(event.TouchInput.X, event.TouchInput.Y);
				m_move_sent_as_mouse_event = false;

				// update shootline (in case the game handles the event we send below)
				m_shootline = m_device
						->getSceneManager()
						->getSceneCollisionManager()
						->getRayFromScreenCoordinates(m_move_downlocation);

				// send a middle click event so the game can handle single touches
				SEvent *translated = new SEvent;
				memset(translated, 0, sizeof(SEvent));
				translated->EventType = EET_MOUSE_INPUT_EVENT;
				translated->MouseInput.X = m_move_downlocation.X;
				translated->MouseInput.Y = m_move_downlocation.Y;
				translated->MouseInput.ButtonStates = EMBSM_MIDDLE; // << important!
				translated->MouseInput.Event = EMIE_MMOUSE_LEFT_UP;
				m_receiver->OnEvent(*translated);
				delete translated;
			}
		}

		m_pointerpos[event.TouchInput.ID] = v2s32(event.TouchInput.X, event.TouchInput.Y);
	} else if (event.TouchInput.Event == ETIE_LEFT_UP) {
		verbosestream << "Up event for pointerid: " << event.TouchInput.ID << std::endl;
		handleReleaseEvent(event.TouchInput.ID);
	} else {
		assert(event.TouchInput.Event == ETIE_MOVED);
		size_t move_idx = event.TouchInput.ID;

		if (m_pointerpos[event.TouchInput.ID] ==
		    v2s32(event.TouchInput.X, event.TouchInput.Y)) {
			return;
		}

		if (m_move_id != -1) {
			if ((event.TouchInput.ID == m_move_id) &&
				(!m_move_sent_as_mouse_event || !g_settings->getBool("touchtarget"))) {

				double distance = sqrt(
						(m_pointerpos[event.TouchInput.ID].X - event.TouchInput.X) *
						(m_pointerpos[event.TouchInput.ID].X - event.TouchInput.X) +
						(m_pointerpos[event.TouchInput.ID].Y - event.TouchInput.Y) *
						(m_pointerpos[event.TouchInput.ID].Y - event.TouchInput.Y));

				if ((distance > g_settings->getU16("touchscreen_threshold")) ||
				    (m_move_has_really_moved)) {
					m_move_has_really_moved = true;
					s32 X = event.TouchInput.X;
					s32 Y = event.TouchInput.Y;

					// update camera_yaw and camera_pitch
					s32 dx = X - m_pointerpos[event.TouchInput.ID].X;
					s32 dy = Y - m_pointerpos[event.TouchInput.ID].Y;

					// adapt to similar behaviour as pc screen
					double d         = g_settings->getFloat("mouse_sensitivity");
					double old_yaw   = m_camera_yaw_change;
					double old_pitch = m_camera_pitch;

					m_camera_yaw_change -= dx * d;
					m_camera_pitch = MYMIN(MYMAX(m_camera_pitch + (dy * d), -180), 180);

					// update shootline
					m_shootline = m_device
							->getSceneManager()
							->getSceneCollisionManager()
							->getRayFromScreenCoordinates(v2s32(X, Y));
					m_pointerpos[event.TouchInput.ID] = v2s32(X, Y);
				}
			} else if ((event.TouchInput.ID == m_move_id) &&
					(m_move_sent_as_mouse_event)) {
				m_shootline = m_device
						->getSceneManager()
						->getSceneCollisionManager()
						->getRayFromScreenCoordinates(
								v2s32(event.TouchInput.X, event.TouchInput.Y));
			}
		} else {
			handleChangedButton(event);
		}
	}
}

void TouchScreenGUI::handleChangedButton(const SEvent &event) {
	for (unsigned int i = 0; i < after_last_element_id; i++) {

		if (m_buttons[i].ids.empty())
			continue;

		for (std::vector<size_t>::iterator iter = m_buttons[i].ids.begin();
				  iter != m_buttons[i].ids.end(); ++iter) {

			if (event.TouchInput.ID == *iter) {

				int current_button_id =
						getButtonID(event.TouchInput.X, event.TouchInput.Y);

				if (current_button_id == i)
					continue;

				// remove old button
				handleButtonEvent((touch_gui_button_id) i, *iter, false);

				if (current_button_id == after_last_element_id)
					return;

				handleButtonEvent((touch_gui_button_id) current_button_id, *iter, true);
				return;

			}
		}
	}

	int current_button_id = getButtonID(event.TouchInput.X, event.TouchInput.Y);

	if (current_button_id == after_last_element_id)
		return;

	button_info *btn = &m_buttons[current_button_id];
	if (std::find(btn->ids.begin(), btn->ids.end(), event.TouchInput.ID)
	    == btn->ids.end()) {
		handleButtonEvent((touch_gui_button_id) current_button_id,
							event.TouchInput.ID, true);
	}
}

// Punch or left click
bool TouchScreenGUI::quickTapDetection() {
	m_key_events[0].down_time = m_key_events[1].down_time;
	m_key_events[0].x         = m_key_events[1].x;
	m_key_events[0].y         = m_key_events[1].y;
/*	m_key_events[1].down_time = m_move_downtime;
	m_key_events[1].x         = m_move_downlocation.X;
	m_key_events[1].y         = m_move_downlocation.Y;

	u64 delta = porting::getDeltaMs(m_key_events[0].down_time, porting::getTimeMs());
	if (delta > 400)
		return false;*/

	// ignore the occasional touch
	u64 delta = porting::getDeltaMs(m_move_downtime, porting::getTimeMs());
	if (delta < 50)
		return false;

	/*double distance = sqrt(
			(m_key_events[0].x - m_key_events[1].x) * (m_key_events[0].x - m_key_events[1].x) +
			(m_key_events[0].y - m_key_events[1].y) * (m_key_events[0].y - m_key_events[1].y));


	if (distance > (20 + g_settings->getU16("touchscreen_threshold")))
		return false;*/

	SEvent* translated = new SEvent();
	memset(translated, 0, sizeof(SEvent));
	translated->EventType               = EET_MOUSE_INPUT_EVENT;
	translated->MouseInput.X            = m_key_events[0].x;
	translated->MouseInput.Y            = m_key_events[0].y;
	translated->MouseInput.Shift        = false;
	translated->MouseInput.Control      = false;
	translated->MouseInput.ButtonStates = EMBSM_RIGHT;

	// update shootline
	m_shootline = m_device
			->getSceneManager()
			->getSceneCollisionManager()
			->getRayFromScreenCoordinates(v2s32(m_key_events[0].x, m_key_events[0].y));

	translated->MouseInput.Event = EMIE_RMOUSE_PRESSED_DOWN;
	verbosestream << "TouchScreenGUI::translateEvent right click press" << std::endl;
	m_receiver->OnEvent(*translated);

	translated->MouseInput.ButtonStates = 0;
	translated->MouseInput.Event        = EMIE_RMOUSE_LEFT_UP;
	verbosestream << "TouchScreenGUI::translateEvent right click release" << std::endl;
	m_receiver->OnEvent(*translated);
	delete translated;
	return true;
}

TouchScreenGUI::~TouchScreenGUI() {
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		button_info *btn = &m_buttons[i];
		if (btn->guibutton != nullptr) {
			btn->guibutton->drop();
			btn->guibutton = nullptr;
		}
	}
}

void TouchScreenGUI::step(float dtime) {
	// simulate keyboard repeats
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		button_info *btn = &m_buttons[i];

		if (!btn->ids.empty()) {
			btn->repeatcounter += dtime;

			if (btn->repeatcounter < btn->repeatdelay) continue;

			btn->repeatcounter              = 0;
			SEvent translated{};
			memset(&translated, 0, sizeof(SEvent));
			translated.EventType            = irr::EET_KEY_INPUT_EVENT;
			translated.KeyInput.Key         = btn->keycode;
			translated.KeyInput.PressedDown = false;
			m_receiver->OnEvent(translated);

			translated.KeyInput.PressedDown = true;
			m_receiver->OnEvent(translated);
		}
	}

	// if a new placed pointer isn't moved for some time start digging
	if ((m_move_id != -1) &&
			(!m_move_has_really_moved) &&
			(!m_move_sent_as_mouse_event)) {

		u64 delta = porting::getDeltaMs(m_move_downtime, porting::getTimeMs());

		if (delta > (MIN_DIG_TIME * 1000.F)) {
			m_shootline = m_device
					->getSceneManager()
					->getSceneCollisionManager()
					->getRayFromScreenCoordinates(
							v2s32(m_move_downlocation.X, m_move_downlocation.Y));

			SEvent translated{};
			memset(&translated, 0, sizeof(SEvent));
			translated.EventType               = EET_MOUSE_INPUT_EVENT;
			translated.MouseInput.X            = m_move_downlocation.X;
			translated.MouseInput.Y            = m_move_downlocation.Y;
			translated.MouseInput.Shift        = false;
			translated.MouseInput.Control      = false;
			translated.MouseInput.ButtonStates = EMBSM_LEFT;
			translated.MouseInput.Event        = EMIE_LMOUSE_PRESSED_DOWN;
			verbosestream << "TouchScreenGUI::step left click press" << std::endl;
			m_receiver->OnEvent(translated);
			m_move_sent_as_mouse_event         = true;
		}
	}
}

void TouchScreenGUI::resetHud() {
	m_hud_rects.clear();
}

void TouchScreenGUI::registerHudItem(int index, const rect<s32> &rect) {
	m_hud_rects[index] = rect;
}

void TouchScreenGUI::Toggle(bool visible) {
	m_visible = visible;
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		button_info *btn = &m_buttons[i];
		if (btn->guibutton != nullptr) {
			btn->guibutton->setVisible(visible);
		}
	}

	// clear all active buttons
	if (!visible) {
		while (!m_known_ids.empty()) {
			handleReleaseEvent(m_known_ids.begin()->id);
		}
	}
}

void TouchScreenGUI::hide() {
	if (!m_visible)
		return;

	Toggle(false);
}

void TouchScreenGUI::show() {
	if (m_visible)
		return;

	Toggle(true);
}
