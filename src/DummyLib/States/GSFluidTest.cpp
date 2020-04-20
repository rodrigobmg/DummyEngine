#include "GSFluidTest.h"

#include <fstream>
#include <memory>

#include <Eng/GameStateManager.h>
#include <Eng/Gui/Image.h>
#include <Eng/Gui/Image9Patch.h>
#include <Eng/Gui/Renderer.h>
#include <Eng/Gui/Utils.h>
#include <Eng/Renderer/Renderer.h>
#include <Eng/Scene/SceneManager.h>
#include <Eng/Utils/Cmdline.h>
#include <Ren/Context.h>
#include <Ren/GL.h>
#include <Ren/Utils.h>
#include <Sys/AssetFile.h>
#include <Sys/Json.h>
#include <Sys/MemBuf.h>
#include <Sys/Time_.h>

#include "../Gui/FontStorage.h"
#include "../Gui/PagedReader.h"
#include "../Viewer.h"

namespace GSFluidTestInternal {
#if defined(__ANDROID__)
const char SCENE_NAME[] = "assets/scenes/"
#else
const char SCENE_NAME[] = "assets_pc/scenes/"
#endif
                          "fluid_test.json";

const int FlGridRes = 128;
} // namespace GSFluidTestInternal

GSFluidTest::GSFluidTest(GameBase *game) : GSBaseState(game) {
    const std::shared_ptr<FontStorage> fonts =
        game->GetComponent<FontStorage>(UI_FONTS_KEY);
    book_main_font_ = fonts->FindFont("book_main_font");
    book_emph_font_ = fonts->FindFont("book_emph_font");
    book_caption_font_ = fonts->FindFont("book_caption_font");
    book_caption_font_->set_scale(1.25f);
}

void GSFluidTest::Enter() {
    using namespace GSFluidTestInternal;

    GSBaseState::Enter();

    log_->Info("GSUITest: Loading scene!");
    GSBaseState::LoadScene(SCENE_NAME);

    /*test_image_.reset(new Gui::Image{
        *ctx_, "assets_pc/textures/test_image.uncompressed.png", Ren::Vec2f{ -0.5f, -0.5f
    }, Ren::Vec2f{ 0.5f, 0.5f }, ui_root_.get()
    });

    test_frame_.reset(new Gui::Image9Patch{
        *ctx_, "assets_pc/textures/ui/frame_01.uncompressed.png", Ren::Vec2f{ 2.0f, 2.0f
    }, 1.0f, Ren::Vec2f{ 0.0f, 0.1f }, Ren::Vec2f{ 0.5f, 0.5f }, ui_root_.get()
    });*/

#if defined(__ANDROID__)
    const char *book_name = "assets/scenes/test/test_book/test_book.json";
#else
    const char *book_name = "assets_pc/scenes/test/test_book/test_book.json";
#endif

    {
        JsObject js_book;

        { // Load book data from file
            Sys::AssetFile in_book(book_name);
            if (!in_book) {
                log_->Error("Can not open book file %s", book_name);
            } else {
                const size_t scene_size = in_book.size();

                std::unique_ptr<uint8_t[]> scene_data(new uint8_t[scene_size]);
                in_book.Read((char *)&scene_data[0], scene_size);

                Sys::MemBuf mem(&scene_data[0], scene_size);
                std::istream in_stream(&mem);

                if (!js_book.Read(in_stream)) {
                    throw std::runtime_error("Cannot load dialog!");
                }
            }
        }
    }

    /*{
        JsObject config;
        config[Gui::GL_DEFINES_KEY] = JsString{""};
        page_renderer_.reset(new Gui::Renderer{*ctx_, config});
    }*/

    // disable bloom and fxaa, they make fonts look bad
    render_flags_ &= ~EnableBloom;
    render_flags_ &= ~EnableFxaa;
}

void GSFluidTest::OnPostloadScene(JsObject &js_scene) {
    using namespace GSFluidTestInternal;

    GSBaseState::OnPostloadScene(js_scene);

    view_dir_ = Ren::Vec3f{0.0f, 0.0f, 1.0f};
    view_fov_ = 45.0f;

    if (js_scene.Has("camera")) {
        const JsObject &js_cam = js_scene.at("camera").as_obj();
        if (js_cam.Has("view_origin")) {
            const JsArray &js_orig = js_cam.at("view_origin").as_arr();
            view_origin_[0] = (float)js_orig.at(0).as_num().val;
            view_origin_[1] = (float)js_orig.at(1).as_num().val;
            view_origin_[2] = (float)js_orig.at(2).as_num().val;
        }

        if (js_cam.Has("view_dir")) {
            const JsArray &js_dir = js_cam.at("view_dir").as_arr();
            view_dir_[0] = (float)js_dir.at(0).as_num().val;
            view_dir_[1] = (float)js_dir.at(1).as_num().val;
            view_dir_[2] = (float)js_dir.at(2).as_num().val;
        }

        /*if (js_cam.Has("fwd_speed")) {
            const JsNumber &js_fwd_speed = (const JsNumber &)js_cam.at("fwd_speed");
            max_fwd_speed_ = (float)js_fwd_speed.val;
        }*/

        if (js_cam.Has("fov")) {
            const JsNumber &js_fov = js_cam.at("fov").as_num();
            view_fov_ = (float)js_fov.val;
        }

        if (js_cam.Has("max_exposure")) {
            const JsNumber &js_max_exposure = js_cam.at("max_exposure").as_num();
            max_exposure_ = (float)js_max_exposure.val;
        }
    }

    plane_index_ = scene_manager_->FindObject("plane");

    fl_grid_.reset(new FlCell[FlGridRes * FlGridRes]);
    std::fill(fl_grid_.get(), fl_grid_.get() + FlGridRes * FlGridRes, FlCell{});

    for (int y = 56; y < 72; y++) {
        for (int x = 56; x < 72; x++) {
            FlCell& cell = fl_grid_[y * FlGridRes + x];
            cell.density = 1.0f;
        }
    }
}

void GSFluidTest::OnUpdateScene() {
    using namespace GSFluidTestInternal;

    GSBaseState::OnUpdateScene();

    const float delta_time_s = fr_info_.delta_time_us * 0.000001f;

    
}

void GSFluidTest::Exit() { GSBaseState::Exit(); }

void GSFluidTest::DrawUI(Gui::Renderer *r, Gui::BaseElement *root) {
    using namespace GSFluidTestInternal;

    /*if (hit_point_screen_.initialized()) {
        paged_reader_->DrawHint(r, hit_point_screen_.GetValue() + Ren::Vec2f{ 0.0f, 0.05f
    }, root);
    }*/

    GSBaseState::DrawUI(r, root);
}

void GSFluidTest::Draw(uint64_t dt_us) {
    using namespace GSFluidTestInternal;

    /*if (book_state_ != eBookState::BkClosed) {
        if (book_state_ == eBookState::BkOpened) {
            if (hit_point_ndc_.initialized()) {
                auto page_root = Gui::RootElement{ Ren::Vec2i{ page_buf_.w, page_buf_.h }
    };

                const int page_base = paged_reader_->cur_page();
                for (int i = 0; i < 2; i++) {
                    paged_reader_->set_cur_page(page_base +
    page_order_indices[(size_t)book_state_][i]);

                    paged_reader_->Resize(2.0f * page_corners_uvs[i * 2] -
    Ren::Vec2f{ 1.0f }, 2.0f * (page_corners_uvs[i * 2 + 1] - page_corners_uvs[i * 2]),
    &page_root); paged_reader_->Press(hit_point_ndc_.GetValue(), true); if
    (paged_reader_->selected_sentence() != -1) { break;
                    }
                }
                paged_reader_->set_cur_page(page_base +
    page_order_indices[(size_t)book_state_][0]);

                hit_point_ndc_.destroy();
            }
        }
        RedrawPages(page_renderer_.get());
    }*/

    const float delta_time_s = fr_info_.delta_time_us * 0.000001f;

    {   // update grid

    }

    const SceneData& scene = scene_manager_->scene_data();

    if (plane_index_ != 0xffffffff) {
        SceneObject* plane = scene_manager_->GetObject(plane_index_);

        uint32_t mask = CompDrawableBit;
        if ((plane->comp_mask & mask) == mask) {
            auto* dr = (Drawable*)scene.comp_store[CompDrawable]->Get(
                plane->components[CompDrawable]);

            Ren::Mesh* mesh = dr->mesh.get();
            Ren::TriGroup& grp = mesh->group(0);

            UpdatePlaneMaterial(grp.mat.get(), delta_time_s);
        }
    }

    auto up_vector = Ren::Vec3f{0.0f, 1.0f, 0.0f};
    if (Ren::Length2(Ren::Cross(view_dir_, up_vector)) < 0.001f) {
        up_vector = Ren::Vec3f{-1.0f, 0.0f, 0.0f};
    }

    const Ren::Vec3f view_origin = view_origin_ + Ren::Vec3f{0.0f, view_offset_, 0.0f};

    scene_manager_->SetupView(view_origin, (view_origin + view_dir_), up_vector,
                              view_fov_, max_exposure_);

    GSBaseState::Draw(dt_us);
}

bool GSFluidTest::HandleInput(const InputManager::Event &evt) {
    using namespace Ren;
    using namespace GSFluidTestInternal;

    // pt switch for touch controls
    if (evt.type == RawInputEvent::EvP1Down || evt.type == RawInputEvent::EvP2Down) {
        if (evt.point.x > (float)ctx_->w() * 0.9f &&
            evt.point.y < (float)ctx_->h() * 0.1f) {
            uint32_t new_time = Sys::GetTimeMs();
            if (new_time - click_time_ < 400) {
                use_pt_ = !use_pt_;
                if (use_pt_) {
                    scene_manager_->InitScene_PT();
                    invalidate_view_ = true;
                }

                click_time_ = 0;
            } else {
                click_time_ = new_time;
            }
        }
    }

    bool input_processed = true;

    switch (evt.type) {
    case RawInputEvent::EvP1Down: {
        const Ren::Vec2f p =
            Gui::MapPointToScreen(Ren::Vec2i{(int)evt.point.x, (int)evt.point.y},
                                  Ren::Vec2i{ctx_->w(), ctx_->h()});
    } break;
    case RawInputEvent::EvP2Down: {

    } break;
    case RawInputEvent::EvP1Up: {
        const Ren::Vec2f p =
            Gui::MapPointToScreen(Ren::Vec2i{(int)evt.point.x, (int)evt.point.y},
                                  Ren::Vec2i{ctx_->w(), ctx_->h()});
    } break;
    case RawInputEvent::EvP2Up: {
    } break;
    case RawInputEvent::EvP1Move: {
        Ren::Vec2f p =
            Gui::MapPointToScreen(Ren::Vec2i{(int)evt.point.x, (int)evt.point.y},
                                  Ren::Vec2i{ctx_->w(), ctx_->h()});
    } break;
    case RawInputEvent::EvP2Move: {

    } break;
    case RawInputEvent::EvKeyDown: {
        input_processed = false;
    } break;
    case RawInputEvent::EvKeyUp: {
        if (evt.key_code == KeyUp || (evt.key_code == KeyW && !cmdline_enabled_)) {
        } else {
            input_processed = false;
        }
    } break;
    case RawInputEvent::EvResize:
        // paged_reader_->Resize(ui_root_.get());
        break;
    default:
        break;
    }

    if (!input_processed) {
        GSBaseState::HandleInput(evt);
    }

    return true;
}

void GSFluidTest::UpdatePlaneMaterial(Ren::Material *plane_mat, float delta_time_s) {
    using namespace GSFluidTestInternal;

    // release texture
    const int ref_count = plane_mat->textures[0]->ref_count();
    plane_mat->textures[0] = {};

    Ren::Texture2DParams params;
    params.w = FlGridRes;
    params.h = FlGridRes;
    params.format = Ren::eTexFormat::RawRGBA32F;
    params.filter = Ren::eTexFilter::NoFilter;
    params.repeat = Ren::eTexRepeat::ClampToEdge;

    Ren::eTexLoadStatus load_status;
    plane_mat->textures[0] =
        ctx_->LoadTexture2D("__plane_texture__", fl_grid_.get(),
                            FlGridRes * FlGridRes * sizeof(FlCell), params, &load_status);
    assert(load_status == Ren::eTexLoadStatus::TexCreatedFromData);

    /*{   // register material
        Ren::eMatLoadStatus status;
        orig_page_mat_ = ctx_->LoadMaterial("book/book_page0.txt", nullptr, &status,
    nullptr, nullptr); if (status != Ren::MatFound) { log_->Error("Failed to find material
    book/book_page0"); return;
        }

        Ren::ProgramRef programs[Ren::MaxMaterialProgramCount];
        for (int i = 0; i < Ren::MaxMaterialProgramCount; i++) {
            programs[i] = orig_page_mat_->program(i);
        }

        Ren::Texture2DRef textures[Ren::MaxMaterialTextureCount];
        for (int i = 0; i < Ren::MaxMaterialTextureCount; i++) {
            textures[i] = orig_page_mat_->texture(i);
        }

        // replace texture
        textures[2] = page_tex_;

        Ren::Vec4f params[Ren::MaxMaterialParamCount];
        for (int i = 0; i < Ren::MaxMaterialParamCount; i++) {
            params[i] = orig_page_mat_->param(i);
        }

        page_mat_ = ctx_->materials().Add("__book_page_material__",
    orig_page_mat_->flags(), programs, textures, params, log_.get());
    }*/
}
