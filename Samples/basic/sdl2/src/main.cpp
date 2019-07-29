/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 Nuno Silva
 * Copyright (c) 2019 The RmlUi Team, and contributors
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

#include <RmlUi/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Debugger/Debugger.h>
#include <Shell.h>
#include <ShellFileInterface.h>

#include "SystemInterfaceSDL2.h"
#include "RenderInterfaceSDL2.h"

#include <SDL.h>

#include <GL/glew.h>

int main(int argc, char **argv)
{
#ifdef RMLUI_PLATFORM_WIN32
        DoAllocConsole();
#endif

        int window_width = 1024;
        int window_height = 768;

    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window * screen = SDL_CreateWindow("LibRmlUi SDL2 test", 20, 20, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glcontext = SDL_GL_CreateContext(screen);
    int oglIdx = -1;
    int nRD = SDL_GetNumRenderDrivers();
    for(int i=0; i<nRD; i++)
    {
        SDL_RendererInfo info;
        if(!SDL_GetRenderDriverInfo(i, &info))
        {
            if(!strcmp(info.name, "opengl"))
            {
                oglIdx = i;
            }
        }
    }
    SDL_Renderer * renderer = SDL_CreateRenderer(screen, oglIdx, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    GLenum err = glewInit();

    if(err != GLEW_OK)
        fprintf(stderr, "GLEW ERROR: %s\n", glewGetErrorString(err));

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, window_width, window_height, 0, 0, 1);
 
	auto Renderer = std::make_shared<RmlUiSDL2Renderer>(renderer, screen);
	auto SystemInterface = std::make_shared<RmlUiSDL2SystemInterface>();
	
	Rml::Core::String root = Shell::FindSamplesRoot();
	auto FileInterface = std::make_shared<ShellFileInterface>(root);

	Rml::Core::SetFileInterface(FileInterface);
	Rml::Core::SetRenderInterface(Renderer);
    Rml::Core::SetSystemInterface(SystemInterface);

	if(!Rml::Core::Initialise())
		return 1;

	Rml::Core::FontDatabase::LoadFontFace("assets/Delicious-Bold.otf");
	Rml::Core::FontDatabase::LoadFontFace("assets/Delicious-BoldItalic.otf");
	Rml::Core::FontDatabase::LoadFontFace("assets/Delicious-Italic.otf");
	Rml::Core::FontDatabase::LoadFontFace("assets/Delicious-Roman.otf");

	Rml::Core::Context *Context = Rml::Core::CreateContext("default",
		Rml::Core::Vector2i(window_width, window_height));

	Rml::Debugger::Initialise(Context);

	Rml::Core::ElementDocument *Document = Context->LoadDocument("assets/demo.rml");

	if(Document)
	{
		Document->Show();
		fprintf(stdout, "\nDocument loaded");
	}
	else
	{
		fprintf(stdout, "\nDocument is NULL");
	}

    bool done = false;

	while(!done)
	{
        SDL_Event event;

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

		Context->Render();
        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_MOUSEMOTION:
                    Context->ProcessMouseMove(event.motion.x, event.motion.y, SystemInterface->GetKeyModifiers());
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    Context->ProcessMouseButtonDown(SystemInterface->TranslateMouseButton(event.button.button), SystemInterface->GetKeyModifiers());
                    break;

                case SDL_MOUSEBUTTONUP:
                    Context->ProcessMouseButtonUp(SystemInterface->TranslateMouseButton(event.button.button), SystemInterface->GetKeyModifiers());
                    break;

                case SDL_MOUSEWHEEL:
                    Context->ProcessMouseWheel(event.wheel.y, SystemInterface->GetKeyModifiers());
                    break;

                case SDL_KEYDOWN:
                {
                    // Intercept F8 key stroke to toggle RmlUi's visual debugger tool
                    if( event.key.keysym.sym == SDLK_F8 )
                    {
                        Rml::Debugger::SetVisible( ! Rml::Debugger::IsVisible() );
                        break;
                    }
                    
                    Context->ProcessKeyDown(SystemInterface->TranslateKey(event.key.keysym.sym), SystemInterface->GetKeyModifiers());
                    break;
                }
                
                default:
                    break;
            }
        }
		Context->Update();
	}

    Rml::Core::Shutdown();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();

	return 0;
};
