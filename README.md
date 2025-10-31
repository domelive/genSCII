# genSCII: ASCII Art Generator in C

A lightweight, terminal-based ASCII art generator written in C that converts images into stylized ASCII representations with optional 256-color support.

## Features

- Image-to-ASCII conversion using customizable character sets
- Terminal-aware sizing: automatically fits output to your terminal dimensions
- Multiple grayscale conversion methods:
  - Luminance-weighted (ITU-R BT.709 standard: 0.2126*R + 0.7152*G + 0.0722*B)
  - Simple average ((R + G + B) / 3)
- Color support:
  - 256-color mode: maps sampled RGB values to ANSI 256-color palette
  - Grayscale-only mode (default)
- Flexible sampling:
  - Point sampling (top-left pixel of region)
  - Average pooling (mean brightness/color over block)
- Aspect ratio correction using configurable terminal character aspect ratio (default: 2.0)
- Cross-platform terminal detection via ioctl (falls back to 80x24 if unavailable)

## Usage
```
./ascii-art-gen --input image.jpg --output output.txt [OPTIONS]
```

### Required Arguments

- -i, --input FILE      : Input image path (JPEG, PNG, etc.)
- -o, --output FILE     : Output ASCII text file path

### Optional Arguments

- -c, --charset SET        : Character set for brightness mapping (default: `@%#*+=-:. `)
- -a, --aspect RATIO       : Terminal character aspect ratio (default: 2.0)
- -g, --gray-method METHOD : Grayscale method: average or luminance (default: luminance)
- -m, --colored MODE       : Color mode: 256 (default: none/grayscale)
- -h, --help               : Show help message

### Examples

Basic grayscale ASCII art
```
./ascii-art-gen -i photo.jpg -o art.txt
```

With custom character set and 256-color support
```
./ascii-art-gen -i cat.png -o colorful_cat.txt -c " .:-=+*#%@" -m 256
```

Using average grayscale and custom aspect ratio
```
./ascii-art-gen -i landscape.jpg -o ascii_landscape.txt -g average -a 1.8
```

Tip: For best results in terminal, use a monospaced font, ensure your terminal supports ANSI 256 colors if using -m 256 and for the best detailed results zoom out the terminal as much as possible.

## Implementation Details

- Image loading/saving: Uses stb_image (public domain)
- Memory management: Explicit allocation tracking (STB_ALLOCATED vs SELF_ALLOCATED)
- Efficient sampling: Region clamping and bounds checking prevent out-of-bounds access
- ANSI 256-color conversion: Uses 6x6x6 RGB cube mapping (16 + 36*r + 6*g + b)
- Modular design: Separation of concerns between Image, Generator, and CLI layers

## Future Roadmap

### Planned Features

- Dithering support:
  - [ ] Floyd-Steinberg error diffusion
  - [ ] Ordered dithering matrices
- Edge/border detection:
  - [ ] Sobel or Canny edge highlighting in ASCII output
  - [ ] Contour-aware character selection
- Interactive terminal mode:
  - [ ] Real-time preview with live terminal resizing
  - [ ] Keyboard controls for zoom/pan
- Extended color support:
  - [x] True color (24-bit ANSI escape sequences)
  - [x] 16-color mode fallback
- Performance optimizations:
  - [ ] SIMD-accelerated sampling
  - [ ] Multithreaded region processing
- Additional output formats:
  - [ ] HTML with embedded styles
  - [ ] SVG vector output

## Dependencies

- Compiler: GCC or Clang (C99 compatible)
- Libraries: None (stb_image is embedded as header-only)
- System: POSIX-compliant OS (Linux/macOS); Windows support via WSL or MinGW
