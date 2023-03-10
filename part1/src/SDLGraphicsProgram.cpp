#include "SDLGraphicsProgram.hpp"
#include "ObjectManager.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// is the program currently being drawn in wireframe mode?
bool wireframe = false;
float rotateSpeed = 0.01f;
int shaderIndex = 1;
int shadingType = 5;

// Initialization function
// Returns a true or false value based on successful completion of setup.
// Takes in dimensions of window.
SDLGraphicsProgram::SDLGraphicsProgram(int w, int h, std::string objectFile, std::string textureFile, std::string normalFile):m_screenWidth(w),m_screenHeight(h){
	// Initialization flag
	bool success = true;
	// String to hold any errors that occur.
	std::stringstream errorStream;
	// The window we'll be rendering to
	m_window = NULL;
	// Render flag

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0){
		errorStream << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		success = false;
	}
	else{
		//Use OpenGL 3.3 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		// We want to request a double buffer for smooth updating.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		//Create window
		m_window = SDL_CreateWindow( "Lab",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                m_screenWidth,
                                m_screenHeight,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

		// Check if Window did not create.
		if( m_window == NULL ){
			errorStream << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
			success = false;
		}

		//Create an OpenGL Graphics Context
		m_openGLContext = SDL_GL_CreateContext( m_window );
		if( m_openGLContext == NULL){
			errorStream << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
			success = false;
		}

		// Initialize GLAD Library
		if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
			errorStream << "Failed to iniitalize GLAD\n";
			success = false;
		}

		//Initialize OpenGL
		if(!InitGL()){
			errorStream << "Unable to initialize OpenGL!\n";
			success = false;
		}
  	}

    // If initialization did not work, then print out a list of errors in the constructor.
    if(!success){
        errorStream << "SDLGraphicsProgram::SDLGraphicsProgram - Failed to initialize!\n";
        std::string errors=errorStream.str();
        SDL_Log("%s\n",errors.c_str());
    }else{
        SDL_Log("SDLGraphicsProgram::SDLGraphicsProgram - No SDL, GLAD, or OpenGL, errors detected during initialization\n\n");
    }

	// SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN); // Uncomment to enable extra debug support!
	GetOpenGLVersionInfo();


	// Setup our objects
    for(int i= 0; i < 1; ++i){ 
        Object* temp = new Object;
		temp->LoadFromFile(objectFile, textureFile, normalFile);
        ObjectManager::Instance().AddObject(temp);
    }

    glShadeModel(GL_SMOOTH);

}


// Proper shutdown of SDL and destroy initialized objects
SDLGraphicsProgram::~SDLGraphicsProgram(){
    // Reclaim all of our objects
    ObjectManager::Instance().RemoveAll();

    //Destroy window
	SDL_DestroyWindow( m_window );
	// Point m_window to NULL to ensure it points to nothing.
	m_window = nullptr;
	//Quit SDL subsystems
	SDL_Quit();
}


// Initialize OpenGL
// Setup any of our shaders here.
bool SDLGraphicsProgram::InitGL(){
	//Success flag
	bool success = true;

	return success;
}


// Update OpenGL
void SDLGraphicsProgram::Update(float speed){
    // Rotate object
    static float rot = 0;
    rot+=speed;
    if(rot>360){rot=0;}

    // Here we hard-code a giant scene
    // Yuck, we'll fix this in a future assignment.
    ObjectManager::Instance().GetObject(0).GetTransform().LoadIdentity();
    // Push back our wall a bit
    ObjectManager::Instance().GetObject(0).GetTransform().Translate(0.0f,0.0f,-2.0f);
    // Rotate on y-axis
    ObjectManager::Instance().GetObject(0).GetTransform().Rotate(rot,0.0f,1.0f,0.0f);
    // Make our wall a little bigger
    ObjectManager::Instance().GetObject(0).GetTransform().Scale(2.0f,2.0f,2.0f);

    // Update all of the objects
    ObjectManager::Instance().UpdateAll(m_screenWidth,m_screenHeight);
}



// Render
// The render function gets called once per loop
void SDLGraphicsProgram::Render(){
	// Setup our OpenGL State machine
    // TODO: Read this
    // The below command is new!
    // What we are doing, is telling opengl to create a depth(or Z-buffer) 
    // for us that is stored every frame.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


    // Initialize clear color
    // This is the background of the screen.
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    glClearColor( 0.2f, 0.2f, 0.2f, 1.f );
    // TODO: Read this
    // Clear color buffer and Depth Buffer
    // Remember that the 'depth buffer' is our
    // z-buffer that figures out how far away items are every frame
    // and we have to do this every frame!
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Nice way to debug your scene in wireframe!
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    // Render all of our objects in a simple loop
    ObjectManager::Instance().RenderAll();

//    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
//    glStencilMask(0x00);
//    glDisable(GL_DEPTH_TEST);
//    ObjectManager::Instance().GetObject(0).getShader(1)->SetUniform1f("outlining",);
//    glUniform1f(glGetUniformLocation(outliningProgram.ID, "outlining"), 1.08f);
//
//    glStencilMask(0xFF);
//    glStencilFunc(ALWAYS, 0, 0xFF);
//    glEnable(GL_DEPTH_TEST);

 
	// Delay to slow things down just a bit!
    SDL_Delay(15);
}


//Loops forever!
void SDLGraphicsProgram::Loop(){
    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();

    // Set the camera speed for how fast we move.
    float cameraSpeed = 0.05f;
    float blendRatio = 0.4f;


    // While application is running
    while(!quit){
        ObjectManager::Instance().GetObject(0).getShader()->SetUniform1f("time", (float)SDL_GetTicks() / 1000.0);
        ObjectManager::Instance().GetObject(0).getShader()->SetUniform1i("shaderIndex", shaderIndex);
        ObjectManager::Instance().GetObject(0).getShader()->SetUniform1f("blend", blendRatio);
     	     	 //Handle events on queue
		while(SDL_PollEvent( &e ) != 0){
        	// User posts an event to quit
	        // An example is hitting the "x" in the corner of the window.
    	    if(e.type == SDL_QUIT){
        		quit = true;
	        }
            // Handle keyboard input for the camera class
            if(e.type==SDL_MOUSEMOTION){
                // Handle mouse movements
                int mouseX = e.motion.x;
                int mouseY = e.motion.y;
                ObjectManager::Instance().GetObject(0).getShader()->SetUniform1f("mousex", mouseX / 1.0f);
                ObjectManager::Instance().GetObject(0).getShader()->SetUniform1f("mousey", mouseY / 1.0f);
                //Camera::Instance().MouseLook(mouseX, mouseY);
            }
            switch(e.type){
                // Handle keyboard presses
                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym){
                        case SDLK_LEFT:
                        //    Camera::Instance().MoveLeft(cameraSpeed);
                            break;
                        case SDLK_RIGHT:
                        //    Camera::Instance().MoveRight(cameraSpeed);
                            break;
                        case SDLK_UP:
                            rotateSpeed += 0.01;
                            break;
                        case SDLK_DOWN:
                            rotateSpeed -= 0.01;
                            break;
                        case SDLK_RSHIFT:
                            blendRatio < 0.5f ? blendRatio += 0.1 : blendRatio;
                            break;
                        case SDLK_LSHIFT:
                            blendRatio > 0.0f ? blendRatio -= 0.1 : blendRatio;
                            break;
                        case SDLK_SPACE:
                            shaderIndex = shaderIndex == 0 ? 1 : 0;
                            break;
                        case SDLK_q:
                            quit = true;
                            break;
                        case SDLK_1:
                            shadingType = 1;
                            break;
                        case SDLK_2:
                            shadingType = 2;
                            break;
                        case SDLK_3:
                            shadingType = 3;
                            break;
                        case SDLK_4:
                            shadingType = 4;
                            break;
                        case SDLK_5:
                            shadingType = 5;
                            break;
                        case SDLK_6:
                            shadingType = 6;
                            break;
                        case SDLK_7:
                            shadingType = 7;
                            break;
                        case SDLK_w:
                            if (wireframe) {
                                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                                wireframe = false;
                            } else {
                                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                wireframe = true;
                            }
                            //    Camera::Instance().MoveDown(cameraSpeed);
                            break;
                    }
                break;
            }

      	} // End SDL_PollEvent loop.
        ObjectManager::Instance().GetObject(0).getShader()->SetUniform1i("type",shadingType);
		// Update our scene
		Update(rotateSpeed);
		// Render using OpenGL
	    Render(); 	// TODO: potentially move this depending on your logic
					// for how you handle drawing a triangle or rectangle.
      	//Update screen of our specified window
      	SDL_GL_SwapWindow(GetSDLWindow());
	}
    //Disable text input
    SDL_StopTextInput();
}



// Get Pointer to Window
SDL_Window* SDLGraphicsProgram::GetSDLWindow(){
  return m_window;
}

// Helper Function to get OpenGL Version Information
void SDLGraphicsProgram::GetOpenGLVersionInfo(){
	SDL_Log("(Note: If you have two GPU's, make sure the correct one is selected)");
	SDL_Log("Vendor: %s",(const char*)glGetString(GL_VENDOR));
	SDL_Log("Renderer: %s",(const char*)glGetString(GL_RENDERER));
	SDL_Log("Version: %s",(const char*)glGetString(GL_VERSION));
	SDL_Log("Shading language: %s",(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}
