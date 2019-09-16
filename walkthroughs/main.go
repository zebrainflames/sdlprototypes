package main

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

func main() {
	err := sdl.Init(sdl.INIT_VIDEO)
	check(err)
	defer sdl.Quit()

	// Create OpenGL accelerated window and set VSYNC on to limit resource usage - otherwise we'll hog up an entire
	// CPU core.
	// TODO: check documentation for sdl.RENDERER_ACCELERATED & OpenGL Rendering
	// TODO: how to limit resource usage when vsync is off? Some sensible target fps?
	window, err := sdl.CreateWindow("SDL testing",
		sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, sdl.WINDOW_SHOWN)
	check(err)
	surface, err := window.GetSurface()
	check(err)
	imgFlags := img.INIT_PNG
	img.Init(imgFlags)
	flower, err := img.Load("walkthroughs/hitman1_machine.png")
	check(err)
	defer flower.Free()
	optimized, err := flower.Convert(flower.Format, 0)
	check(err)
	defer optimized.Free()
	stretch := &sdl.Rect{
		X: 0,
		Y: 0,
		W: ScreenWidth,
		H: ScreenHeight,
	}

	err = optimized.BlitScaled(nil, surface, stretch)
	check(err)

	err = window.UpdateSurface()
	check(err)

	//main game loop
	for quit := false; !quit; quit = processEvents() {

	}

	err = window.Destroy()
	check(err)
}