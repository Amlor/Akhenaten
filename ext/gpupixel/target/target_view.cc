/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include "target_view.h"
#include "../core/gpupixel_context.h"
#include "../utils/util.h"
#include "../filter/filter.h"

USING_NS_GPUPIXEL

TargetView::TargetView()
    : _viewWidth(0),
    _viewHeight(0),
    _fillMode(FillMode::PreserveAspectRatio),
    _displayProgram(0),
    _positionAttribLocation(0),
    _texCoordAttribLocation(0),
    _colorMapUniformLocation(0) {
    _backgroundColor = { 0.f, 0.f, 0.f, 0.f };
    init();
}

TargetView::~TargetView() {
    delete _displayProgram;
    _displayProgram = nullptr;
}

void TargetView::init() {
    _displayProgram = GLProgram::createByShaderString(kDefaultVertexShader, kDefaultFragmentShader);
    _positionAttribLocation = _displayProgram->getAttribLocation("position");
    _texCoordAttribLocation = _displayProgram->getAttribLocation("inputTextureCoordinate");
    _colorMapUniformLocation = _displayProgram->getUniformLocation("textureCoordinate");
    GPUPixelContext::getInstance()->setActiveShaderProgram(_displayProgram);
    CHECK_GL(glEnableVertexAttribArray(_positionAttribLocation));
    CHECK_GL(glEnableVertexAttribArray(_texCoordAttribLocation));
};

void TargetView::setInputFramebuffer(
    std::shared_ptr<Framebuffer> framebuffer,
    RotationMode rotationMode /* = NoRotation*/,
    int texIdx /* = 0*/) {
    std::shared_ptr<Framebuffer> lastInputFramebuffer;
    RotationMode lastInputRotation = NoRotation;
    if (_inputFramebuffers.find(0) != _inputFramebuffers.end()) {
        lastInputFramebuffer = _inputFramebuffers[0].frameBuffer;
        lastInputRotation = _inputFramebuffers[0].rotationMode;
    }

    Target::setInputFramebuffer(framebuffer, rotationMode, texIdx);

    if (lastInputFramebuffer != framebuffer && framebuffer &&
        (!lastInputFramebuffer ||
        !(lastInputFramebuffer->getWidth() == framebuffer->getWidth() &&
        lastInputFramebuffer->getHeight() == framebuffer->getHeight() &&
        lastInputRotation == rotationMode))) {
        _updateDisplayVertices();
    }
}

void TargetView::setFillMode(FillMode fillMode) {
    if (_fillMode != fillMode) {
        _fillMode = fillMode;
        _updateDisplayVertices();
    }
}

void TargetView::setMirror(bool mirror) {
    if (_mirror != mirror) {
        _mirror = mirror;
    }
}

void TargetView::onSizeChanged(int width, int height) {
    if (_viewWidth != width || _viewHeight != height) {
        _viewWidth = width;
        _viewHeight = height;
        _framebuffer = GPUPixelContext::getInstance()->getFramebufferCache()->fetchFramebuffer(_viewWidth, _viewHeight, false);
        _capturedFrameData.resize(_viewWidth * _viewHeight * 4);
        _updateDisplayVertices();
    }
}

int testindex = 0;
#include "../utils/stb_image_write.h"

void TargetView::update(int64_t frameTime) {
    onSizeChanged(_inputFramebuffers[0].frameBuffer->getWidth(), _inputFramebuffers[0].frameBuffer->getHeight());

    GPUPixelContext::getInstance()->setActiveShaderProgram(_displayProgram);
    _framebuffer->active();

    CHECK_GL(glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a));
    CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    CHECK_GL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL(glBindTexture(GL_TEXTURE_2D, _inputFramebuffers[0].frameBuffer->getTexture()));
    CHECK_GL(glUniform1i(_colorMapUniformLocation, 0));
    CHECK_GL(glVertexAttribPointer(_positionAttribLocation, 2, GL_FLOAT, 0, 0, _displayVertices));
    CHECK_GL(glVertexAttribPointer(_texCoordAttribLocation, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(_inputFramebuffers[0].rotationMode)));
    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    CHECK_GL(glReadBuffer(GL_COLOR_ATTACHMENT0));
    CHECK_GL(glReadPixels(0, 0, _framebuffer->getWidth(), _framebuffer->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, _capturedFrameData.data()));
    //char bmpname[32] = { 0 };
    //snprintf(bmpname, 32, "gtgimg%u.bmp", testindex);
    //stbi_write_bmp(bmpname, _framebuffer->getWidth(), _framebuffer->getHeight(), 4, (const void *)_capturedFrameData);

    _framebuffer->inactive();
}

void TargetView::_updateDisplayVertices() {
    if (_inputFramebuffers.find(0) == _inputFramebuffers.end() ||
        _inputFramebuffers[0].frameBuffer == 0) {
        return;
    }

    std::shared_ptr<Framebuffer> inputFramebuffer = _inputFramebuffers[0].frameBuffer;
    RotationMode inputRotation = _inputFramebuffers[0].rotationMode;

    int rotatedFramebufferWidth = inputFramebuffer->getWidth();
    int rotatedFramebufferHeight = inputFramebuffer->getHeight();
    if (rotationSwapsSize(inputRotation)) {
        rotatedFramebufferWidth = inputFramebuffer->getHeight();
        rotatedFramebufferHeight = inputFramebuffer->getWidth();
    }

    float framebufferAspectRatio = rotatedFramebufferHeight / (float)rotatedFramebufferWidth;
    float viewAspectRatio = _viewHeight / (float)_viewWidth;

    float insetFramebufferWidth = 0.0;
    float insetFramebufferHeight = 0.0;
    if (framebufferAspectRatio > viewAspectRatio) {
        insetFramebufferWidth = _viewHeight / (float)rotatedFramebufferHeight * rotatedFramebufferWidth;
        insetFramebufferHeight = _viewHeight;
    } else {
        insetFramebufferWidth = _viewWidth;
        insetFramebufferHeight = _viewWidth / (float)rotatedFramebufferWidth * rotatedFramebufferHeight;
    }

    float scaledWidth = 1.0;
    float scaledHeight = 1.0;

    if (_fillMode == FillMode::PreserveAspectRatio) {
        scaledWidth = insetFramebufferWidth / _viewWidth;
        scaledHeight = insetFramebufferHeight / _viewHeight;
    } else if (_fillMode == FillMode::PreserveAspectRatioAndFill) {
        scaledWidth = _viewWidth / insetFramebufferHeight;
        scaledHeight = _viewHeight / insetFramebufferWidth;
    }

    _displayVertices[0] = -scaledWidth;
    _displayVertices[1] = -scaledHeight;
    _displayVertices[2] = scaledWidth;
    _displayVertices[3] = -scaledHeight;
    _displayVertices[4] = -scaledWidth;
    _displayVertices[5] = scaledHeight;
    _displayVertices[6] = scaledWidth;
    _displayVertices[7] = scaledHeight;
}

const GLfloat *TargetView::_getTexureCoordinate(RotationMode rotationMode) {
    static const GLfloat noRotationTextureCoordinates[] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
    };

    static const GLfloat rotateRightTextureCoordinates[] = {
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      0.0f, 0.0f,
    };

    static const GLfloat rotateLeftTextureCoordinates[] = {
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      1.0f, 1.0f,
    };

    static const GLfloat verticalFlipTextureCoordinates[] = {
      0.0f, 1.0f,
      1.0f, 1.0f,
      0.0f, 0.0f,
      1.0f, 0.0f,
    };

    static const GLfloat horizontalFlipTextureCoordinates[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
    };

    static const GLfloat rotateRightVerticalFlipTextureCoordinates[] = {
      1.0f, 0.0f,
      1.0f, 1.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
    };

    static const GLfloat rotateRightHorizontalFlipTextureCoordinates[] = {
      0.0f, 1.0f,
      0.0f, 0.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
    };

    static const GLfloat rotate180TextureCoordinates[] = {
      1.0f, 0.0f,
      0.0f, 0.0f,
      1.0f, 1.0f,
      0.0f, 1.0f,
    };

    switch (rotationMode) {
    default:
    case NoRotation:
    {
        if (_mirror) {
            return horizontalFlipTextureCoordinates;
        } else {
            return noRotationTextureCoordinates;
        }
    }
    case RotateLeft:
        return rotateLeftTextureCoordinates;
    case RotateRight:
        return rotateRightTextureCoordinates;
    case FlipVertical:
        return verticalFlipTextureCoordinates;
    case FlipHorizontal:
        return horizontalFlipTextureCoordinates;
    case RotateRightFlipVertical:
        return rotateRightVerticalFlipTextureCoordinates;
    case RotateRightFlipHorizontal:
        return rotateRightHorizontalFlipTextureCoordinates;
    case Rotate180:
        return rotate180TextureCoordinates;
    }
}
