#ifndef SHADOWHELPER_H
#define SHADOWHELPER_H

#include <QMap>
#include <QSharedPointer>
#include <QPixmap>
#include <KDecoration2/DecorationShadow>

namespace UKUI {

class Decoration;

class ShadowHelper
{
    friend class Decoration;
public:
    enum State {
        Active,
        Inactive,
        None
    };

    static ShadowHelper *globalInstance();

    QSharedPointer<KDecoration2::DecorationShadow> getShadow(State state);

private:
    ShadowHelper();
    QPixmap getShadowPixmap(State state);

    QMap<State, QSharedPointer<KDecoration2::DecorationShadow>> m_shadows;
};

}

#endif // SHADOWHELPER_H
