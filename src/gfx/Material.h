#ifndef MATERIAL_H
#define MATERIAL_H

// TODO: add ambient/diffuse/specular/shininess? Rename?
class Material {
public:
    virtual ~Material() = default;
    virtual void use() = 0;
    virtual void stopUsing() = 0;
};


#endif //MATERIAL_H
