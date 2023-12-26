//
// Created by Beejoe Xing on 2023/11/15.
//

#ifndef OPENMAPLECLIENT_TEMPLATE_H
#define OPENMAPLECLIENT_TEMPLATE_H

#endif //OPENMAPLECLIENT_TEMPLATE_H

template <typename T>
T lerp(const T& a, const T& b, float t) {
    return a + t * (b - a);
}
