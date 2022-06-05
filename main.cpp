#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include <api.h>
#include <show.h>
#include <utils.h>
#include <chrono>
#include <scene.h>

constexpr int depth = 255;
constexpr int width = 1000;
constexpr int height = 1000;

Vec3f light_pos = Vec3f{-1,1,1};
Vec3f light_dir = Vec3f{-1,1,1}.normalized();
Vec3f camera{1,1,4};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

Scene* sceneRef;

Vec3f* cameraPos = new Vec3f[3]{Vec3f{1,1,4}, Vec3f{1.41f,0,4}, Vec3f{-1,1,4}};
int cameraIdx = 0;

void key_callback(window_t *window, keycode_t key, int pressed) {
	if(pressed == 1){
		if(key == KEY_SPACE){
        	// window->should_close = true;
			std::cout << "SPACE pressed" << std::endl;
    	}else if(key == KEY_A) {
			std::cout << "A pressed" << std::endl;
			cameraIdx = (cameraIdx + 1) % 3 ;
			sceneRef->GetCamera().SetPosition(cameraPos[cameraIdx]);
			sceneRef->Render();
			std::cout << "Re Render Over"  << std::endl;
		}
	}
}

void test_show_keys_input(window_t* window) {
	printf("%d %d %d %d %d \n",window->keys[0],
        window->keys[1],
        window->keys[2],
        window->keys[3],
        window->keys[4]
    );
}

int main(int argc, char **argv)
{
	const char* outputDir;
	if (3 == argc){
		outputDir = argv[2];
	}else{
		std::cerr << "Function main(): 3 args expected, " << argc <<" args provided !" << std::endl;
		exit(-1);
	}

	Scene mainScene(Camera(camera,up,center), 0, new Model(argv[1]),depth,width,height,
		Vec2i({0,0}),Light(light_pos,light_dir),outputDir);
	sceneRef = &mainScene;

	mainScene.Render();

	std::cout<<"Render Over!"<<std::endl;

	window_t* win = window_create("Render test", width, height);

	image_t * targetImg = image_create(width, height, 3);
	targetImg->buffer = mainScene.GetScreenBufferData();
	callbacks_t callbacks;
	callbacks.button_callback = nullptr;
	callbacks.key_callback = key_callback;
	callbacks.scroll_callback = nullptr;
	input_set_callbacks(win, callbacks);
	
	auto startTime = std::chrono::steady_clock().now();

	while(!window_should_close(win)) {

		startTime = std::chrono::steady_clock().now();
		window_draw_image(win, targetImg);
		// test_show_keys_input(win);
		input_poll_events();
		
		// auto deltaTime = std::chrono::steady_clock().now() - startTime;
		// std::cout << "Delta Time: " << 
		// 	std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime).count() << std::endl;
	}

	window_destroy(win);
	
	return 0;
}
