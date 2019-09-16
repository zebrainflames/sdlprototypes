package main

import (
	"fmt"
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
		panic(err)
	}
}

func main() {
	err := sdl.Init(sdl.INIT_VIDEO)
	check(err)
	defer sdl.Quit()

	// Create OpenGL accelerated window and set VSYNC on to limit resource usage - otherwise we'll hog up an entire
	// CPU core.
	// TODO: check documentation for sdl.RENDERER_ACCELERATED
	// TODO: how to limit resource usage when vsync is off? Some sensible target fps?
	window, err := sdl.CreateWindow("SDL testing", sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, 0)
	check(err)
	renderer, err := sdl.CreateRenderer(window, 0, sdl.RENDERER_ACCELERATED)
	check(err)
	err = sdl.GLSetSwapInterval(1)
	check(err)


	tex, err := renderer.CreateTexture(sdl.PIXELFORMAT_ARGB8888,sdl.TEXTUREACCESS_STATIC,ScreenWidth, ScreenHeight)
	check(err)

	pixels := make([]byte, 4*ScreenWidth*ScreenHeight)
	for i := range pixels {
		pixels[i] = 0xFF
	}

	//main game loop
	quit := false

	mouseButtonDown := false
	for !quit {
		var (
			x int32
			y int32
		)
		for event := sdl.PollEvent(); event != nil; event = sdl.PollEvent() {

			switch t := event.(type) {
			case *sdl.QuitEvent:
				fmt.Println("Quitting game")
				quit = true
				goto cleanup
			case *sdl.MouseButtonEvent:
				fmt.Printf("[%d ms] MouseButton\ttype:%d\tid:%d\tx:%d\ty:%d\tbutton:%d\tstate:%d\n",
					t.Timestamp, t.Type, t.Which, t.X, t.Y, t.Button, t.State)
				if t.State == 1 {
					// draw...
					fmt.Println("Drawing!")
					mouseButtonDown = true
					x, y = t.X, t.Y
				} else {
					mouseButtonDown = false
				}
			case *sdl.MouseMotionEvent:
				x = t.X
				y = t.Y
			}

			if mouseButtonDown {
				setPixelsToBlack(pixels, x, y, 10)
			}
		}


		err = tex.Update(nil, pixels, ScreenWidth * 4)
		check(err)

		err = renderer.Clear()
		check(err)
		err = renderer.Copy(tex, nil, nil)
		check(err)
		renderer.Present()
	}

	// non-deferrable cleanup:
cleanup:
	tex.Destroy()
	err = renderer.Destroy()
	check(err)
	err = window.Destroy()
	check(err)
}

func setPixelsToBlack(pixels []byte, x int32, y int32, brushsize int32) {
	xmin := maxi(0, x - brushsize / 2)
	xmax := mini(x + brushsize / 2, ScreenWidth)
	ymin := maxi(0, y - brushsize / 2)
	ymax := mini(y + brushsize / 2, ScreenHeight)
	for y := ymin; y <= ymax; y++ {
		for x := xmin; x < xmax; x++ {
			i := getPixelIndex(x,y)
			pixels[i] = 0x00
			pixels[i + 1] = 0x00
			pixels[i + 2] = 0x00
			pixels[i + 3] = 0x00
		}
	}
}

func mini(a, b int32) int32 {
	if a < b {
		return a
	}
	return b
}

func maxi(a, b int32) int32 {
	if a > b {
		return a
	}
	return b
}

func getPixelIndex(x,y int32) int32 {
	return 4*(y * ScreenWidth + x)
}
