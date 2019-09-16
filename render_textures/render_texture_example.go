package render_textures

import (
	"fmt"
	log "github.com/mgutz/logxi/v1"
	"github.com/veandco/go-sdl2/img"
	"github.com/veandco/go-sdl2/sdl"
	"runtime"
)

const (
	ScreenWidth = 640
	ScreenHeight = 480
)

func init() {
	runtime.LockOSThread()
}

func check(err error) {
	if err != nil {
		fmt.Println(err)
		fmt.Println(sdl.GetError())
		log.Fatal("Exit!")
	}
}

func processEvents() bool {
	for event := sdl.PollEvent(); event != nil; event = sdl.PollEvent() {
		switch t := event.(type) {
		case *sdl.QuitEvent:
			fmt.Println("Quitting game")
			return true
		case *sdl.MouseButtonEvent:
			fmt.Printf("[%d ms] MouseButton\ttype:%d\tid:%d\tx:%d\ty:%d\tbutton:%d\tstate:%d\n",
				t.Timestamp, t.Type, t.Which, t.X, t.Y, t.Button, t.State)
		case *sdl.KeyboardEvent:
			fmt.Printf("[%d ms] KeyboardEvent\ttype:%d\tstate:%d\tkeysym:%v\trepeat:%d\n",
				t.Timestamp, t.Type, t.State,t.Keysym,t.Repeat)
			if t.Keysym.Sym == sdl.K_q {
				fmt.Println("Quit pressed!")
				return true
			}
		}
	}
	return false
}

func loadOptimizedSurface(path string) *sdl.Surface {
	raw, err := img.Load(path)
	check(err)
	defer raw.Free()
	optimized, err := raw.Convert(raw.Format, 0)
	check(err)
	return optimized
}



func main() {
	err := sdl.Init(sdl.INIT_VIDEO)
	check(err)
	defer sdl.Quit()


	// TODO: check documentation for sdl.RENDERER_ACCELERATED & OpenGL Rendering
	// TODO: how to limit resource usage when vsync is off? Some sensible target fps?
	// TODO: what's included in the sdl.WINDOW_SHOWN specification..?
	window, err := sdl.CreateWindow("SDL testing",
		sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, sdl.WINDOW_SHOWN)
	check(err)



	img.Init(img.INIT_PNG)
	renderer, err := sdl.CreateRenderer(window, -1, sdl.RENDERER_ACCELERATED)
	check(err)
	err = renderer.SetDrawColor(0xFF,0xFF,0xFF,0xFF)
	check(err)
	// Create OpenGL accelerated window and set VSYNC on to limit resource usage - otherwise we'll hog up an entire
	// CPU core.
	// NOTE: Has to happen after getting an OpenGL context, e.g by creating the renderer above.
	err = sdl.GLSetSwapInterval(1)
	check(err)

	raw, err := img.Load("walkthroughs/hitman1_machine.png")
	check(err)
	tex, err := renderer.CreateTextureFromSurface(raw)
	check(err)
	raw.Free()

	//main game loop
	for quit := false; !quit; quit = processEvents() {
		renderer.Clear()
		renderer.Copy(tex, nil, nil)
		renderer.Present()
	}
	err = tex.Destroy()
	check(err)

	err = window.Destroy()
	check(err)
}