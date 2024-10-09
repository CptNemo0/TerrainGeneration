#ifndef CLOTH_H
#define CLOTH_H

class Cloth
{
public:
    Cloth(float offset, int size);
    void Update(float dt);
    void Draw();
};

#endif // !CLOTH_H