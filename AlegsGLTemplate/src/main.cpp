#include <GL/glew.h>
#include <glm/matrix.hpp>
#include <iostream>
#include <cassert>
#include <SDL2\SDL.h>
#include <fstream>
#include <Windows.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

typedef std::string String;
SDL_Window* window = nullptr;
SDL_GLContext glContext;
bool quit = false;
FT_Library library;
GLuint VBO;
GLuint ebo;
GLuint program;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
glm::mat4 mvp = glm::ortho(0.f, 1280.f, 720.f, 0.f, -1.f, 1.f);
int TextureWidth = 0;
int TextureHeight = 0;

GLuint loadShader(const String& resourcePath) {
	std::string vertexTemp;
	std::string fragmentTemp;

	std::ifstream vertexFile;
	std::ifstream fragmentFile;

	GLuint program;

	vertexFile.exceptions(std::ifstream::badbit);
	fragmentFile.exceptions(std::ifstream::badbit);

	try
	{
		vertexFile.open(resourcePath + ".vertex");
		fragmentFile.open(resourcePath + ".fragment");

		std::stringstream vertexStream;
		std::stringstream fragmentStream;

		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();

		vertexFile.close();
		fragmentFile.close();

		vertexTemp = vertexStream.str();
		fragmentTemp = fragmentStream.str();
	}
	catch (std::ifstream::failure error)
	{
		std::cout << "ERROR: Reading shader " << std::endl;
	}

	const GLchar* vertexCode = vertexTemp.c_str();
	const GLchar* fragmentCode = fragmentTemp.c_str();

	GLuint vertex;
	GLuint fragment;
	GLint success;
	GLchar infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexCode, nullptr);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "ERROR: Shader compilation: " << std::endl << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentCode, nullptr);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		std::cout << "ERROR: Shader compilation: " << std::endl << infoLog << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "ERROR: Program linking: " << std::endl << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return program;
}

static std::string platformGetFontPath(const String& faceName) {
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::wstring wsFaceName(faceName.begin(), faceName.end());

	// Open Windows font registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		throw std::runtime_error("Cant open registry!");
	}

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);

	if (result != ERROR_SUCCESS) {
		throw std::runtime_error("Cant query registry!");
	}

	DWORD valueIndex = 0;
	LPWSTR valueName = new WCHAR[maxValueNameSize];
	LPBYTE valueData = new BYTE[maxValueDataSize];
	DWORD valueNameSize, valueDataSize, valueType;
	std::wstring wsFontFile;

	// Look for a matching font name
	do {

		wsFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;

		result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

		valueIndex++;

		if (result != ERROR_SUCCESS || valueType != REG_SZ) {
			continue;
		}

		std::wstring wsValueName(valueName, valueNameSize);

		// Found a match
		if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.size()) == 0) {
			wsFontFile.assign((LPWSTR)valueData, valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	delete[] valueName;
	delete[] valueData;

	RegCloseKey(hKey);

	if (wsFontFile.empty()) {
		throw std::runtime_error("Did not find matching font!");
	}

	// Build full font file path
	WCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);

	std::wstringstream ss;
	ss << winDir << "\\Fonts\\" << wsFontFile;
	wsFontFile = ss.str();

	return String(wsFontFile.begin(), wsFontFile.end());
}

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	// Set OpenGL attributes.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Create the window. Use data from the config.
	window = SDL_CreateWindow(
		"unnamed",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1280,
		720, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);


	glContext = SDL_GL_CreateContext(window); // Create context.

	// Initialise glew and TTF.
	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0.2f, 0.4f, 0.6f, 1.0f); // Set clear color.

	// Enable GL stuff.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void update();
void draw();
void initFreetype();

int main(int argc, char** argv) {
	init();
	initFreetype();
	program = loadShader("asd");
	while (!quit) {
		SDL_Event evnt;
		while (SDL_PollEvent(&evnt)) {
			switch (evnt.type) {
			case SDL_KEYDOWN:
				switch (evnt.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = true;
					break;
				}
				break;
			}
		}
		update();
		draw();
		SDL_Delay(16);
	}

	SDL_Quit();
	FT_Done_FreeType(library);
	return 0;
}
struct Vec2
{
	Vec2() { }
	Vec2(float a, float b)
		: x(a), y(b) { }

	float x, y;
};
union Pixel32
{
	Pixel32() : b(0), r(0), g(0), a(0) {}
	Pixel32(uint8 bi, uint8 gi, uint8 ri, uint8 ai = 255)
	{
		b = bi;
		g = gi;
		r = ri;
		a = ai;
	}

	struct
	{
		uint8 r, g, b, a;
	};
};

struct Rect
{
	Rect() { }
	Rect(float left, float top, float right, float bottom)
		: xmin(left), xmax(right), ymin(top), ymax(bottom) { }

	void Include(const Vec2 &r)
	{
		xmin = std::min(xmin, r.x);
		ymin = std::min(ymin, r.y);
		xmax = std::max(xmax, r.x);
		ymax = std::max(ymax, r.y);
	}

	float Width() const { return xmax - xmin + 1; }
	float Height() const { return ymax - ymin + 1; }

	float xmin, xmax, ymin, ymax;
};

FT_Face loadFont(String& faceName, uint32 size) {
	String filePath(platformGetFontPath(faceName));

	FT_Face face;
	FT_New_Face(library, filePath.c_str(), 0, &face);
	FT_Set_Char_Size(face, size << 6, size << 6, 90, 90);
	return face;
}

struct Span
{
	Span() { }
	Span(int _x, int _y, int _width, int _coverage)
		: x(_x), y(_y), width(_width), coverage(_coverage) { }

	int x, y, width, coverage;
};

void
RasterCallback(const int y,
const int count,
const FT_Span * const spans,
void * const user)
{
	std::vector<Span> *sptr = (std::vector<Span> *)user;
	for (int i = 0; i < count; ++i)
		sptr->push_back(Span(spans[i].x, y, spans[i].len, spans[i].coverage));
}

void renderSpans(FT_Outline* outline, std::vector<Span>* spans) {
	FT_Raster_Params params;
	memset(&params, 0, sizeof(params));
	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = RasterCallback;
	params.user = spans;

	FT_Outline_Render(library, outline, &params);
}
struct GlyphWrapper {
	Pixel32* pixels;
	int width;
	int height;
	float bearinX;
	float bearingY;
	float advance;
	float x;
	float y;
};


void flippaa(Pixel32* pxl, int w, int h) {
	for (int r = 0; r < (h / 2); r++)
	{
		for (int c = 0; c != w; ++c)
		{
			std::swap(pxl[r * w + c], pxl[(h - 1 - r) * w + c]);
		}
	}
}

std::map<char, GlyphWrapper> glyphs;
void initFreetype() {
	FT_Init_FreeType(&library);
	FT_Face face = loadFont(String("Arial"), 200);

	char start = 'a';
	char end = 'g';
	float outlineWidth = 3.f;
	float offX = 0;
	float offY = 0;
	for (char ch = start; ch <= end; ch++) {
		FT_UInt gindex = FT_Get_Char_Index(face, ch);
		if (FT_Load_Glyph(face, gindex, FT_LOAD_DEFAULT)) throw std::runtime_error("Failed!");
		if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) throw std::runtime_error("asd");
		// Need an outline for this to work.
		//if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) throw std::runtime_error("No outline");

		std::vector<Span> spans;
		renderSpans(&face->glyph->outline, &spans);

		std::vector<Span> outlineSpans;

		// setup stroker
		FT_Stroker stroker;
		FT_Stroker_New(library, &stroker);

		FT_Stroker_Set(stroker,
			(int)(outlineWidth * 64),
			FT_STROKER_LINECAP_ROUND,
			FT_STROKER_LINEJOIN_ROUND,
			0);

		FT_Glyph glyph;
		if (FT_Get_Glyph(face->glyph, &glyph)) throw std::runtime_error("Cant get glyph");

		FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
	//	if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) throw std::runtime_error("Not outline");

		// render the outline spans to the span list
		FT_Outline* o = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
		renderSpans(o, &outlineSpans);

		// Clean up afterwards.
		FT_Stroker_Done(stroker);
		FT_Done_Glyph(glyph);

		// Now we need to put it all together.
		if (spans.empty()) continue;

		Rect rect(
			spans.front().x,
			spans.front().y,
			spans.front().x,
			spans.front().y);
		for (auto& s : spans) {
			rect.Include(Vec2(s.x, s.y));
			rect.Include(Vec2(s.x + s.width - 1, s.y));
		}

		for (auto& s : outlineSpans) {
			rect.Include(Vec2(s.x, s.y));
			rect.Include(Vec2(s.x + s.width - 1, s.y));
		}
		
		// HOW TO?
		float bearingX = face->glyph->metrics.horiBearingX >> 6; // left
		float bearingY = face->glyph->metrics.horiBearingY >> 6; // top
		float advance = face->glyph->advance.x >> 6; // offset
		assert(bearingX == face->glyph->bitmap_left);
		assert(bearingY == face->glyph->bitmap_top);
		// Get some metrics of our image.
		int imgWidth = rect.Width(),
			imgHeight = rect.Height(),
			imgSize = imgWidth * imgHeight;

		// Allocate data for our image and clear it out to transparent.
		Pixel32 *pxl = new Pixel32[imgSize];
		memset(pxl, 0, sizeof(Pixel32) * imgSize);

		for (Span& s : outlineSpans) {
			for (int w = 0; w < s.width; ++w) {
				pxl[(int)((imgHeight - 1 - (s.y - rect.ymin)) * imgWidth
					+ s.x - rect.xmin + w)]
					= Pixel32(255, 0, 0, s.coverage); // outline color
			}
		}

		for (Span& s : spans) {
			for (int w = 0; w < s.width; ++w) {
				Pixel32 &dst =
					pxl[(int)((imgHeight - 1 - (s.y - rect.ymin)) * imgWidth
					+ s.x - rect.xmin + w)];

				Pixel32 src = Pixel32(0, 255, 0, s.coverage); // font color
				dst.r = (int)(dst.r + ((src.r - dst.r) * src.a) / 255.0f);
				dst.g = (int)(dst.g + ((src.g - dst.g) * src.a) / 255.0f);
				dst.b = (int)(dst.b + ((src.b - dst.b) * src.a) / 255.0f);
				dst.a = std::min(255, dst.a + src.a);
			}
		}
		flippaa(pxl, imgWidth, imgHeight);

		glyphs[ch] = (GlyphWrapper{
			pxl,
			imgWidth,
			imgHeight,
			bearingX,
			bearingY,
			advance,
			offX,
			offY
		});
		offX += imgWidth;
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width = 0;
	int height = 0;
	for (auto& g : glyphs) {
		width += g.second.width;
		height = std::max(height, g.second.height);
	}
	TextureWidth = width;
	TextureHeight = height;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	int x = 0;
	for (auto& g : glyphs) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g.second.width, g.second.height, GL_RGBA, GL_UNSIGNED_BYTE, g.second.pixels);
		x += g.second.width;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 100 sprites, 4 points per sprite, 4 floats 
	glBufferData(GL_ARRAY_BUFFER, 100 * 4 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,	4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	
	// Create an element array

	glGenBuffers(1, &ebo);
	std::vector<unsigned short> indices;
	indices.reserve(100 * 6);
	for (unsigned short i = 0; i < 100 * 6; i += 4) {
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 2);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


int renderText(std::string& text, float x, float y) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	int offset = 0;
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); ++c) {
		GlyphWrapper ch = glyphs[*c];

		float xpos = x + ch.bearinX ;
		float ypos = y - float(ch.bearingY) + TextureHeight;
		float w = ch.width;
		float h = ch.height;

		struct Vertex {
			float x;
			float y;
			float u;
			float v;
		};
		
		// top left x
		float s0 = ch.x / (float)TextureWidth;
		// top left y
		float t0 = ch.y / (float)TextureHeight;
		// bottom right x
		float s1 = (ch.x + ch.width) / (float)TextureWidth;
		// bottom right y
		float t1 = (ch.y + ch.height) / (float)TextureHeight;
		Vertex vertices[4];
		//vertices[0] = Vertex{
		//	xpos,
		//	ypos + h,
		//	s0,
		//	t0
		//};
		//
		//// top left
		//vertices[1] = Vertex{
		//	xpos,
		//	ypos,
		//	s0,
		//	t1
		//};
		//vertices[2] = Vertex{
		//	xpos + w,
		//	ypos,
		//	s1,
		//	t1
		//};

		//vertices[3] = Vertex{
		//	xpos + w,
		//	ypos + h,
		//	s1,
		//	t0
		//};
		vertices[0] = Vertex{
			xpos,
			ypos,
			s0,
			t1
		};
		vertices[1] = Vertex{
			xpos + w,
			ypos,
			s1,
			t1
		};
		vertices[2] = Vertex{
			xpos,
			ypos + h,
			s0,
			t0
		};
		vertices[3] = Vertex{
			xpos + w,
			ypos + h,
			s1,
			t0
		};

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4 * offset, sizeof(vertices), vertices);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
/*		GLuint FBO;
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		
		GLuint t;
		glGenTextures(1, &t);
		glBindTexture(GL_TEXTURE_2D, t);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t, 0);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindTexture(GL_TEXTURE_2D, 1);
		glDrawArrays(GL_TRIANGLES, 0, 6);*/
		offset++;
		x += ch.advance;
	}
	return offset;
}

void update() {
	
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1);
	GLuint mvpLocation = glGetUniformLocation(program, "mvp");
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
	int cnt = renderText(std::string("adfg"), 200, 0);
	glDrawElements(GL_TRIANGLES, 6 * cnt, GL_UNSIGNED_SHORT, 0);
	SDL_GL_SwapWindow(window);
}