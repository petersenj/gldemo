#include "renderer.h"

#include "util.h"
#include "input.h"
#include "model.h"

#include <glad/glad.h>
#include <linmath.h>
#include <math.h>

#define FIELD_OF_VIEW 65.0

GLuint programA;
vbo *cube;
GLint locationModel, locationView, locationProjection, locationTime;

static GLuint makeShader(GLenum shaderType, string fileName) {
    DEBUG("Compiling shader '%s'", fileName.str);

    string shaderSource = readFile(fileName);
    if (!shaderSource.str) {
        FAIL("Can't read '%s'", fileName.str);
    }

    GLuint shader = glCreateShader(shaderType);
    const int length = (int) shaderSource.len;
    glShaderSource(shader, 1, &shaderSource.str, &length);
    glCompileShader(shader);

    FREESTR(shaderSource);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus) {
        //if (!GLAD_GL_VERSION_4_3) {
        //    GLint shaderLogLength;
        //    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shaderLogLength);
        //
        //    if (shaderLogLength > 0) {
        //        char *shaderLog = malloc(shaderLogLength);
        //        if (!shaderLog) {
        //            FAIL("Couldn't allocate memory for shader info log");
        //        }
        //        glGetShaderInfoLog(shader, shaderLogLength, NULL, shaderLog);
        //
        //        WARN("[GLSL] Shader '%s' log: %s", fileName.str, shaderLog);
        //
        //        free(shaderLog);
        //    }
        //}

        FAIL("Shader '%s' did not compile", fileName.str);
    }

    return shader;
}

static void makeProgram(GLuint program) {
    DEBUG("Linking program");

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        //if (!GLAD_GL_VERSION_4_3) {
        //    GLint programLogLength;
        //    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &programLogLength);
        //
        //    if (programLogLength > 0) {
        //        char *programLog = malloc(programLogLength);
        //        if (!programLog) {
        //            FAIL("Couldn't allocate memory for program info log");
        //        }
        //        glGetProgramInfoLog(program, programLogLength, NULL, programLog);
        //
        //        WARN("[GLSL] Program log: %s", programLog);
        //
        //        free(programLog);
        //    }
        //}

        FAIL("Program did not link");
    }
}

void setupRenderer(uint32_t argc, char *argv[]) {
    // Basic options
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Don't really know what this is needed for yet
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (argc < 1) {
        FAIL("No shader name supplied");
    }
    string vertexFileName = strprintf("shader/%s.vert", argv[0]);
    string fragmentFileName = strprintf("shader/%s.frag", argv[0]);

    programA = glCreateProgram();
    glAttachShader(programA, makeShader(GL_VERTEX_SHADER, vertexFileName));
    glAttachShader(programA, makeShader(GL_FRAGMENT_SHADER, fragmentFileName));
    makeProgram(programA);

    FREESTR(fragmentFileName);
    FREESTR(vertexFileName);

    // Get uniform locations in our program, since we don't use GL_ARB_explicit_uniform_location
    locationModel = glGetUniformLocation(programA, "model");
    locationView = glGetUniformLocation(programA, "view");
    locationProjection = glGetUniformLocation(programA, "projection");
    locationTime = glGetUniformLocation(programA, "time");

    // Upload and describe data
    model *cubeModel = loadModel(STR("model/cube.model"));
    cube = uploadModel(cubeModel);
    freeModel(cubeModel);

    glUseProgram(programA);
}

void onViewport(int width, int height) {
    float ratio = width / (float) height;

    // Set up projection matrix
    mat4x4 projection;
    mat4x4_perspective(projection, FIELD_OF_VIEW / 180.0 * PI, ratio, 1.0f, 100.0f);
    glUniformMatrix4fv(locationProjection, 1, GL_FALSE, (const float *) projection);
}

void render(double time) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1f(locationTime, (float) time);

    // Upload view matrix from input
    glUniformMatrix4fv(locationView, 1, GL_FALSE, (const float *) view);

    // glValidateProgram(programA); ...
    glUseProgram(programA);

    for (int i = 0; i < 8; i++) {
        mat4x4 temp, model;
        mat4x4_translate(temp, sin(i / 8.0 * 2.0 * PI) * 4.0, 0.0, cos(i / 8.0 * 2.0 * PI) * 4.0);
        mat4x4_rotate_Y(model, temp, (time * (i + 1) * 15.0) / 180.0 * PI);
        glUniformMatrix4fv(locationModel, 1, GL_FALSE, (const float *) model);

        drawVBO(cube);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
