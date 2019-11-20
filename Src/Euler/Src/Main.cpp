#include <exception>
#include <fstream>
#include <iostream>

#include <agz/utility/string.h>
#include <D3D/Base.h>

#include <ImGui/imgui.h>
#include <ImGui/imfilebrowser.h>

#include <EulerOperation/Create.h>
#include <EulerOperation/Operation.h>
#include <EulerOperation/Renderer.h>

/**
 * @brief 从文件中读取多边形数据并创建对应的实体
 *
 * 出错时抛异常，故返回值绝不为空
 */
EulerOpr::Solid *CreateSolidFromPolygon2DFile(const std::string &filename)
{
    EulerOpr::Polygon2D polygon;

    std::ifstream fin(filename, std::ifstream::in);
    if(!fin)
    {
        throw std::runtime_error("failed to open file: " + filename);
    }

    // 读取扫成距离

    float height;
    fin >> height;
    if(!fin)
    {
        throw std::runtime_error("failed to parse height value");
    }

    if(height <= 0)
    {
        throw std::runtime_error("invalid height value: " + std::to_string(height));
    }

    // loop记录当前正在追加数据的多边形loop
    std::vector<EulerOpr::Vec2> *loop = &polygon.outerLoop;

    std::string line;
    while(std::getline(fin, line))
    {
        // 跳过空行和注释

        if(agz::stdstr::starts_with(line, "#"))
        {
            continue;
        }

        line = agz::stdstr::trim(line);
        if(line.empty())
        {
            continue;
        }

        // 遇到$时开始下一个inner loop

        if(agz::stdstr::starts_with(line, "$"))
        {
            if(loop->size() < 3)
            {
                throw std::runtime_error("polygon loop vertex count must be greater or equal to 3");
            }
            polygon.innerLoops.emplace_back();
            loop = &polygon.innerLoops.back();

            continue;
        }

        EulerOpr::Vec2 point;

        std::stringstream sst;
        sst << agz::stdstr::trim(line);
        sst >> point.x >> point.y;
        if(!sst)
        {
            throw std::runtime_error("invalid line: " + line);
        }

        loop->push_back(point);
    }

    if(loop->size() < 3)
    {
        throw std::runtime_error("polygon loop vertex count must be greater or equal to 3");
    }

    return CreateFromPolygon(polygon, height);
}

void Run()
{
    using namespace D3D;
    using namespace EulerOpr;

    // 创建窗口和事件管理器

    WindowDesc windowDesc;
    windowDesc.windowTitle  = L"CAD Project By Guan Z";
    windowDesc.clientWidth  = 1024;
    windowDesc.clientHeight = 640;

    Window window;
    window.Initialize(windowDesc);

    KeyboardEventManager *keyboard = window.GetKeyboard();
    MouseEventManager    *mouse    = window.GetMouse();

    // 初始化文件浏览器，用于选择多边形数据文件

    ImGui::FileBrowser fileBrowser;
    fileBrowser.SetTitle("select polygon file");

    // 创建预置的实体模型

    Polygon2D polygon;
    polygon.outerLoop =
    {
        { -2, -2 },
        { -2, +2 },
        { +2, +2 },
        { +2, -2 }
    };
    polygon.innerLoops.push_back({
        { 0, -1 },
        { 1, 0 },
        { 0, 1 },
        { -1, 0 }
    });
    polygon.innerLoops.push_back({
        { 0, -1.5f },
        { 1.5f, 0 },
        { 1.5f, -1.5f }
    });

    Solid *solid = CreateFromPolygon(polygon, 2);

    // 初始化实体渲染器

    Renderer renderer;
    renderer.SetSolid(solid);

    // 渲染配置

    bool wireframeMode = false;

    float horiRad        = 0;
    float vertRad        = 0.5f;
    float cameraDistance = 10.0f;

    WheelScrollHandler wheelScrollHandler([&](const WheelScrollEvent &e)
    {
        cameraDistance -= 0.4f * e.offset;
        cameraDistance = agz::math::clamp(cameraDistance, 0.5f, 20.0f);
    });
    mouse->Attach(&wheelScrollHandler);

    // 主循环

    while(!window.GetCloseFlag())
    {
        // 处理窗口事件

        window.DoEvents();
        window.WaitForFocus();
        window.ImGuiNewFrame();

        if(keyboard->IsKeyPressed(KEY_ESCAPE))
        {
            window.SetCloseFlag(true);
        }

        if(mouse->IsMouseButtonPressed(MouseButton::Right))
        {
            vertRad += 0.0065f * mouse->GetRelativeCursorPositionY();
            horiRad -= 0.0065f * mouse->GetRelativeCursorPositionX();

            vertRad = agz::math::clamp(vertRad, -agz::math::PI_f / 2 + 0.02f, agz::math::PI_f / 2 - 0.02f);
        }

        // 计算摄像机位置

        Vec3 cameraPosition =
            cameraDistance * Vec3(
                std::cos(horiRad) * std::cos(vertRad),
                std::sin(vertRad),
                std::sin(horiRad) * std::cos(vertRad));
        Mat4 view = Trans4::look_at(
            cameraPosition,
            { 0, 0, 0 },
            { 0, 1, 0 });
        Mat4 proj = Trans4::perspective(
            agz::math::deg2rad(60.0f),
            window.GetClientAspectRatio(),
            0.1f, 100.0f);

        // 绘制实体

        window.ClearDefaultRenderTarget(0.2f, 0.2f, 0.2f, 0);
        window.ClearDefaultDepthStencil();

        if(wireframeMode)
        {
            renderer.RenderWireframe(view * proj);
        }
        else
        {
            renderer.RenderFace(view * proj);
        }

        // 显示设置菜单

        if(ImGui::Begin("Setting", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Checkbox("Wireframe", &wireframeMode);
            if(ImGui::Button("read polygon from file"))
            {
                fileBrowser.Open();
            }
        }
        ImGui::End();

        fileBrowser.Display();

        // 处理从文件中加载多边形的请求

        if(fileBrowser.HasSelected())
        {
            try
            {
                Solid *newSolid = CreateSolidFromPolygon2DFile(
                    fileBrowser.GetSelected().string());
                DestroySolid(solid);
                solid = newSolid;
                renderer.SetSolid(newSolid);
            }
            catch(const std::exception &err)
            {
                std::cout << err.what() << std::endl;
            }

            fileBrowser.ClearSelected();
        }

        window.ImGuiRender();
        window.SwapBuffers();
    }

    DestroySolid(solid);
}

// 用于检查内存泄露
#include <crtdbg.h>

int main()
{
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
    }
}
