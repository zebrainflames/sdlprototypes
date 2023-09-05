package main

import (
	"fmt"
	"math"
	"math/rand"
	"runtime"
	"sync"
	"time"

	"github.com/veandco/go-sdl2/sdl"
)

const (
	ScreenWidth  = 1200
	ScreenHeight = 800
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
	err := sdl.Init(sdl.INIT_EVERYTHING)
	check(err)
	defer sdl.Quit()

	// Create OpenGL accelerated window and set VSYNC on to limit resource usage - otherwise we'll hog up an entire
	// CPU core.
	// TODO: check documentation for sdl.RENDERER_ACCELERATED
	// TODO: how to limit resource usage when vsync is off? Some sensible target fps?
	window, err := sdl.CreateWindow("SDL testing", sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, sdl.WINDOW_SHOWN|sdl.WINDOW_VULKAN)
	check(err)
	renderer, err := sdl.CreateRenderer(window, 0, sdl.RENDERER_ACCELERATED)
	check(err)
	//err = sdl.GLSetSwapInterval(1)
	//check(err)

	//tex, err := renderer.CreateTexture(sdl.PIXELFORMAT_RGBA8888, sdl.TEXTUREACCESS_STREAMING, ScreenWidth, ScreenHeight)
	//check(err)

	pixels := make([]byte, 4*ScreenWidth*ScreenHeight)
	for i := range pixels {
		pixels[i] = 0x22
	}

	surface, err := sdl.CreateRGBSurfaceWithFormat(0, ScreenWidth, ScreenHeight, 32, sdl.PIXELFORMAT_RGBA8888)
	check(err)
	defer surface.Free()

	tex, err := renderer.CreateTextureFromSurface(surface)
	check(err)
	surface.Free()
	//tex.Query()

	//main game loop
	quit := false

	mouseButtonDown := false
	t := time.Now()
	accum := 0.0
	frames := 0
	fps := 60.0

	for !quit {
		var wg sync.WaitGroup
		wg.Add(1)
		go func(pixels []byte, wgrp *sync.WaitGroup) {
			updatePixels(pixels)
			wgrp.Done()
		}(pixels, &wg)
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
			} /* else {
				updatePixels(pixels)
			}*/
		}
		// update pure go side things first before event polling & what not take us into CGO-land?
		frames++
		dt := time.Since(t).Seconds()
		t = time.Now()
		accum += dt
		if accum > 1.0 {
			fps = float64(frames) / accum
			accum = 0.0
			frames = 0
			fmt.Println("FPS:", fps)
		}

		/*
			if !mouseButtonDown {
				updatePixels(pixels)
			}*/

		// wait for pixel updating goroutines to finish:
		wg.Wait()
		err = tex.Update(nil, pixels, ScreenWidth*4)

		check(err)

		frameCounter++

		err = renderer.Clear()
		check(err)
		err = renderer.Copy(tex, nil, nil)
		check(err)
		renderer.Present()

		window.GLSwap()
	}

	// non-deferrable cleanup:
cleanup:
	tex.Destroy()
	err = renderer.Destroy()
	check(err)
	err = window.Destroy()
	check(err)
}

var frameCounter = 0

// randByte returns a random byte value between 0 - 255
func randByte() byte {
	//return byte(rand.Intn(256))
	if rand.Float32() < 0.5 {
		return 0x22
	}
	return 0x54
}

func updatePixels(pixels []byte) {
	/*for i := range pixels {
		if i%2 == 0 {
			continue
		}
		pixels[i] = 0xD0
	}*/
	var updateFromTo = func(pixels []byte, from, to int) {
		for i := from; i < to; i += 4 {
			//set everything excep alpha to random value for grayscale brightness changes per pixel
			val := byte(math.Abs(math.Sin(float64(frameCounter)/100.0)) * 255)
			pixels[i] = val
			pixels[i+1] = randByte()
			pixels[i+2] = val
			//pixels[i+3] = 0x22
		}
	}
	//updateFromTo(pixels, 0, len(pixels))
	// Split the work to four goroutines and have each of them use updateFromTo
	var wg sync.WaitGroup
	const numGoroutines = 12
	for i := 0; i < numGoroutines; i++ {
		wg.Add(1)
		go func(pixels []byte, from, to int, wgrp *sync.WaitGroup) {
			updateFromTo(pixels, from, to)
			wgrp.Done()
		}(pixels, i*len(pixels)/numGoroutines, (i+1)*len(pixels)/numGoroutines, &wg)
	}
	wg.Wait()
}

func setPixelsToBlack(pixels []byte, x int32, y int32, brushsize int32) {
	xmin := maxi(0, x-brushsize/2)
	xmax := mini(x+brushsize/2, ScreenWidth)
	ymin := maxi(0, y-brushsize/2)
	ymax := mini(y+brushsize/2, ScreenHeight)
	for y := ymin; y <= ymax; y++ {
		for x := xmin; x < xmax; x++ {
			i := getPixelIndex(x, y)
			pixels[i] = 0xF8
			pixels[i+1] = 0xFF
			pixels[i+2] = 0x45
			pixels[i+3] = 0xFF
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

func getPixelIndex(x, y int32) int32 {
	return 4 * (y*ScreenWidth + x)
}
