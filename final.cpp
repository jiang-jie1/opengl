#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <opencv2/opencv.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <cmath>
#include <vector>
//这个是用来将文字着入图片的
class ChineseTextRenderer {
private:
    FT_Library library;
    FT_Face face;

public:
    ChineseTextRenderer(const std::string& fontPath) {
        // 初始化FreeType库
        if (FT_Init_FreeType(&library)) {
            throw std::runtime_error("Could not init FreeType library");
        }

        // 加载字体文件
        if (FT_New_Face(library, fontPath.c_str(), 0, &face)) {
            throw std::runtime_error("Could not load font file");
        }
    }

    ~ChineseTextRenderer() {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void renderText(cv::Mat& image, const std::wstring& text,
        cv::Point position, int fontSize, cv::Scalar color) {
        // 设置字体大小
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        int x = position.x;
        int y = position.y;

        for (wchar_t ch : text) {
            // 加载字符字形
            if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) {
                continue;
            }

            FT_Bitmap* bitmap = &face->glyph->bitmap;

            // 绘制字符
            for (int row = 0; row < bitmap->rows; ++row) {
                for (int col = 0; col < bitmap->width; ++col) {
                    int imgX = x + col + face->glyph->bitmap_left;
                    int imgY = y + row - face->glyph->bitmap_top + fontSize;

                    if (imgX >= 0 && imgX < image.cols &&
                        imgY >= 0 && imgY < image.rows) {
                        unsigned char pixel = bitmap->buffer[row * bitmap->width + col];
                        if (pixel > 0) {
                            float alpha = pixel / 255.0f;
                            cv::Vec3b& imgPixel = image.at<cv::Vec3b>(imgY, imgX);
                            imgPixel[0] = cv::saturate_cast<uchar>(imgPixel[0] * (1 - alpha) + color[0] * alpha);
                            imgPixel[1] = cv::saturate_cast<uchar>(imgPixel[1] * (1 - alpha) + color[1] * alpha);
                            imgPixel[2] = cv::saturate_cast<uchar>(imgPixel[2] * (1 - alpha) + color[2] * alpha);
                        }
                    }
                }
            }

            // 移动到下一个字符位置
            x += face->glyph->advance.x >> 6;
        }
    }
};

// 顶点着色器源码
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// 片段着色器源码
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    
    uniform sampler2D texture1;
    
    void main()
    {
        FragColor = texture(texture1, TexCoord);
    }
)";

// 立方体顶点数据
float vertices[] = {
    // 位置              // 纹理坐标
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// 模型变换参数
float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;
float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;

// 窗口大小
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

int main()
{
    try {
        cv::Mat image = cv::imread("F:/Project1/Project1/resources/images/image1.jpg");
        if (image.empty()) {
            std::cerr << "Could not open or find the image!" << std::endl;
            return -1;
        }

        // 创建中文渲染器
        ChineseTextRenderer renderer("F:/Project1/Project1/resources/msyh.ttf");

        // 使用宽字符字符串表示中文
        std::wstring chineseText = L"蒋洁 2023302111195 计算机学院";

        // 渲染中文文本
        renderer.renderText(image, chineseText, cv::Point(50, 100),
            36, cv::Scalar(255, 105, 180));

        cv::imwrite("F:/Project1/Project1/resources/images/image1.jpg", image);
        /*cv::imshow("Result", image);
        cv::waitKey(0);*/
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Cube with Texture", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 设置顶点缓冲对象和顶点数组对象
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 加载纹理
    unsigned int textures[6];
    std::vector<std::string> texturePaths = {
        "F:/Project1/Project1/resources/images/image1.jpg", "F:/Project1/Project1/resources/images/image2.jpg", "F:/Project1/Project1/resources/images/image3.jpg",
        "F:/Project1/Project1/resources/images/image4.jpg", "F:/Project1/Project1/resources/images/image5.jpg", "F:/Project1/Project1/resources/images/image6.jpg"
    };

    for (int i = 0; i < 6; i++) {
        textures[i] = loadTexture(texturePaths[i].c_str());
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 输入处理
        processInput(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器程序
        glUseProgram(shaderProgram);

        // 创建变换矩阵
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(translateX, translateY, translateZ));
        model = glm::rotate(model, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(scaleX, scaleY, scaleZ));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // 传递变换矩阵到着色器
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

        // 渲染立方体
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 6; i++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }

        // 交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// 处理输入
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 平移控制
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        translateY += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        translateY -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        translateX -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        translateX += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        translateZ -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        translateZ += 0.01f;

    // 旋转控制
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        rotateX += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        rotateX -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        rotateY -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        rotateY += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        rotateZ += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        rotateZ -= 1.0f;

    // 缩放控制
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        scaleX += 0.01f;
        scaleY += 0.01f;
        scaleZ += 0.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        scaleX -= 0.01f;
        scaleY -= 0.01f;
        scaleZ -= 0.01f;
    }

    // 重置变换
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        translateX = translateY = translateZ = 0.0f;
        rotateX = rotateY = rotateZ = 0.0f;
        scaleX = scaleY = scaleZ = 1.0f;
    }
}

// 窗口大小调整回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

// 加载纹理函数
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
//控制方式
//平移 :
//
//W / S : Y轴上下移动
//
//A / D : X轴左右移动
//
//Q / E : Z轴前后移动
//
//旋转 :
//
//I / K : 绕X轴旋转
//
//J / L : 绕Y轴旋转
//
//U / O : 绕Z轴旋转
//
//缩放 :
//
//Z: 放大
//
//X : 缩小
//
//重置 :
//
//R: 重置所有变换