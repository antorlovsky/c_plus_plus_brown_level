#include "Common.h"

using namespace std;

class Rectangle : public IShape {
public:
    Rectangle() = default;
    Rectangle(Point pos, Size size, shared_ptr<ITexture> texture)
        : position_(pos), size_(size), texture_(move(texture)) {}

    unique_ptr<IShape> Clone() const override {
        return make_unique<Rectangle>(position_, size_, texture_);
    }

    void SetPosition(Point pos) override {
        position_ = pos;
    }
    Point GetPosition() const override {
        return position_;
    }

    void SetSize(Size size) override {
        size_ = size;
    }
    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> texture) override {
        texture_ = move(texture);
    }
    ITexture* GetTexture() const override {
        return texture_.get();
    }

    void Draw(Image& image) const override {
        for (int i = 0; i < size_.height; ++i) {
            for (int j = 0; j < size_.width; ++j) {
                if (i + position_.y >= image.size() || j + position_.x >= image[i].size())
                    continue;

                if (texture_ && i < texture_->GetSize().height && j < texture_->GetSize().width) {
                    const Image& texture = texture_->GetImage();

                    if (i < texture.size() && j < texture[i].size()) {
                         image[i + position_.y][j + position_.x] = texture[i][j];
                    }
                }
                else {
                    image[i + position_.y][j + position_.x] = '.';
                }
            }
        }
    }

private:
    Point position_ = {0, 0};
    Size size_ = {0, 0};
    shared_ptr<ITexture> texture_;
};

class Ellipse : public IShape {
public:
    Ellipse() = default;
    Ellipse(Point pos, Size size, shared_ptr<ITexture> texture)
        : position_(pos), size_(size), texture_(move(texture)) {}

    unique_ptr<IShape> Clone() const override {
        return make_unique<Ellipse>(position_, size_, texture_);
    }

    void SetPosition(Point pos) override {
        position_ = pos;
    }
    Point GetPosition() const override {
        return position_;
    }

    void SetSize(Size size) override {
        size_ = size;
    }
    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> texture) override {
        texture_ = move(texture);
    }
    ITexture* GetTexture() const override {
        return texture_.get();
    }

    void Draw(Image& image) const override {
        for (int i = 0; i < size_.height; ++i) {
            for (int j = 0; j < size_.width; ++j) {
                if (!IsPointInEllipse({j, i}, size_)
                    || i + position_.y >= image.size() || j + position_.x >= image[i].size()
                )
                    continue;

                if (texture_ && i < texture_->GetSize().height && j < texture_->GetSize().width) {
                    const Image& texture = texture_->GetImage();

                    if (i < texture.size() && j < texture[i].size()) {
                        image[i + position_.y][j + position_.x] = texture[i][j];
                    }
                }
                else {
                    image[i + position_.y][j + position_.x] = '.';
                }
            }
        }
    }

private:
    Point position_ = {0, 0};
    Size size_ = {0, 0};
    shared_ptr<ITexture> texture_;
};

unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    switch (shape_type) {
        case ShapeType::Rectangle:
            return make_unique<Rectangle>();

        case ShapeType::Ellipse:
            return make_unique<Ellipse>();
    }

    return nullptr;
}