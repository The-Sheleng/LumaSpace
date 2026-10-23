#include "app/product_info.h"

#include <QTest>

class ProductInfoTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void identityIsStable()
    {
        QCOMPARE(lumaspace::productDisplayName(), QStringLiteral("LumaSpace"));
        QCOMPARE(lumaspace::kApplicationIdentifier.toString(),
                 QStringLiteral("com.lumaspace.app"));
    }
};

QTEST_APPLESS_MAIN(ProductInfoTest)

#include "product_info_test.moc"
