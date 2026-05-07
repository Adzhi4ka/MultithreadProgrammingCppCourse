#include "domain/models/acl-level.h"
#include "presentation/widgets/file-tree-widget.h"

#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

namespace tests::client {

    namespace models = ::client::domain::models;
    namespace ui = ::client::presentation;

    TEST(ClientUiTest, FileTreeWidget_SetFiles_StoresFilesAndEmitsSelectionChanged) {
        ui::FileTreeWidget tree;
        QSignalSpy selectionSpy{&tree, &ui::FileTreeWidget::selectedFileChanged};

        std::vector<models::RemoteFile> files{
            models::RemoteFile{
                .id = 10,
                .fullLogicalName = QStringLiteral("/docs/a.txt"),
                .currentVersionId = 100,
                .maxVersionCount = 5,
                .createdAt = 111,
                .createdBy = 7,
                .aclLevel = models::AclLevel::ReadWrite,
            },
            models::RemoteFile{
                .id = 11,
                .fullLogicalName = QStringLiteral("/docs/b.txt"),
                .currentVersionId = 101,
                .maxVersionCount = 5,
                .createdAt = 112,
                .createdBy = 8,
                .aclLevel = models::AclLevel::ReadOnly,
            },
        };

        tree.setFiles(files);

        EXPECT_EQ(tree.fileCount(), 2);
        EXPECT_GE(selectionSpy.count(), 1);

        const auto file = tree.fileById(10);
        ASSERT_TRUE(file.has_value());
        EXPECT_EQ(file->fullLogicalName, QStringLiteral("/docs/a.txt"));
        EXPECT_EQ(file->aclLevel, models::AclLevel::ReadWrite);
    }

    TEST(ClientUiTest, FileTreeWidget_SelectAndDoubleClick_EmitsActivatedFileId) {
        ui::FileTreeWidget tree;

        tree.setFiles({
            models::RemoteFile{
                .id = 42,
                .fullLogicalName = QStringLiteral("/docs/report.txt"),
                .currentVersionId = 100,
                .maxVersionCount = 5,
                .createdAt = 111,
                .createdBy = 7,
                .aclLevel = models::AclLevel::ReadWrite,
            },
        });

        tree.expandAll();

        const auto items = tree.findItems(QStringLiteral("report.txt"), Qt::MatchRecursive, 0);
        ASSERT_EQ(items.size(), 1);

        auto* item = items.front();
        tree.setCurrentItem(item);

        const auto selectedId = tree.selectedFileId();
        ASSERT_TRUE(selectedId.has_value());
        EXPECT_EQ(*selectedId, 42);

        QSignalSpy activationSpy{&tree, &ui::FileTreeWidget::fileActivated};

        const auto itemRect = tree.visualItemRect(item);
        ASSERT_TRUE(itemRect.isValid());

        QTest::mouseClick(tree.viewport(), Qt::LeftButton, Qt::NoModifier, itemRect.center());
        QTest::mouseDClick(tree.viewport(), Qt::LeftButton, Qt::NoModifier, itemRect.center());

        ASSERT_EQ(activationSpy.count(), 1);
        EXPECT_EQ(activationSpy.takeFirst().at(0).toLongLong(), 42);
    }

} // namespace tests::client
