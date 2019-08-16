#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <Eng/GameBase.h>
#include <Eng/GameState.h>
#include <Gui/BaseElement.h>
#include <Gui/BitmapFont.h>
#include <Ren/Camera.h>
#include <Ren/Mesh.h>
#include <Ren/MVec.h>
#include <Ren/Program.h>
#include <Ren/Texture.h>
#include <Ren/SW/SW.h>

#include "GSBaseState.h"

class Cmdline;
class DebugInfoUI;
class GameStateManager;
class FontStorage;
class SceneManager;
class TextPrinter;

class GSUITest2 : public GSBaseState {
    uint64_t last_frame_time_ = 0;
    double cur_fps_ = 0.0;

    uint32_t click_time_ = 0;

    std::shared_ptr<Gui::BitmapFont> dialog_font_;
    float test_time_counter_s = 0.0f;

    std::unique_ptr<Gui::Image>         test_image_, test_image2_, test_image3_, test_image4_;
    std::unique_ptr<Gui::Image9Patch>   test_frame_, test_frame2_, test_frame3_, test_frame4_;
    //std::unique_ptr<TextPrinter>         text_printer_;
    bool                                 is_visible_ = false;

    uint32_t zenith_index_          = 0xffffffff;

    void OnPostloadScene(JsObject &js_scene) override;

    void OnUpdateScene() override;

    void DrawUI(Gui::Renderer *r, Gui::BaseElement *root) override;
public:
    explicit GSUITest2(GameBase *game);
    ~GSUITest2() final = default;

    void Enter() override;
    void Exit() override;

    bool HandleInput(const InputManager::Event &evt) override;
};