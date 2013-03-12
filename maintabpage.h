#ifndef MAINTABPAGE_H
#define MAINTABPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <sharedstashlistwidget.h>
#include <infinitestashitemslistwidget.h>
#include <optioncollection.h>
#include <optionkeys.h>
#include <groupstable.h>
#include <QLabel>
#include <QList>
#include <torchlight2stash.h>
#include <torchlight2stashconverter.h>
#include <settingstabpage.h>

class MainTabPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainTabPage(QWidget *parent = 0);

    ~MainTabPage();

    void SetTorchlight2SharedStashItemsListWidget(SharedStashListWidget* inListWidget)
    {
        mTorchlight2SharedStashItemsListWidget = inListWidget;

        if (mTorchlight2SharedStashItemsListWidget != NULL)
        {
            connect(mTorchlight2SharedStashItemsListWidget,
                    SIGNAL(itemAdded(QListWidgetItem*)), this,
                    SLOT(OnTorchlight2SharedStashItemAdded(QListWidgetItem*)));

            connect(mTorchlight2SharedStashItemsListWidget,
                    SIGNAL(itemsAboutToBeRemoved(QList<QListWidgetItem*>)), this,
                    SLOT(OnTorchlight2SharedStashItemsRemoved(QList<QListWidgetItem*>)));
        }
    }

    void SetInfiniteStashItemsListWidget(InfiniteStashItemsListWidget* inListWidget)
    {
        mInfiniteStashItemsListWidget = inListWidget;
    }

    void SetGroupsComboBox(QComboBox* inComboBox)
    {
        mGroupsComboBox = inComboBox;

        FillGroupsComboBox();
    }

    void SetNumberOfItemsInSharedStashLabel(QLabel* inLabel)
    {
        mNumberOfItemsInSharedStashLabel = inLabel;
    }

    void SetGroupsTable(GroupsTable* inGroupsTable)
    {
        mGroupsTable = inGroupsTable;

        FillGroupsComboBox();
    }

    void SetSettingsTabPage(SettingsTabPage* inSettingsTabPage)
    {
        mSettingsTabPage = inSettingsTabPage;

        if (mSettingsTabPage != NULL)
        {
            connect(mSettingsTabPage, SIGNAL(torchlight2SharedStashFileChanged(QString)), this,
                    SLOT(OnTorchlight2SharedStashFileChanged(QString)));
        }
    }

    void Options(OptionCollection* inOptions)
    {
        mOptions = inOptions;

        if (mOptions != NULL)
        {
            QString torchlight2StashFile = mOptions->Get(OptionKeys::Torchlight2SharedStashFile);
            QString infiniteStashFolder = mOptions->Get(OptionKeys::StashManagerFolder);

            FillSharedStashList(torchlight2StashFile);
        }

    }
    
signals:
    
public slots:


private slots:
    void OnTorchlight2SharedStashItemAdded(QListWidgetItem* item);
    void OnTorchlight2SharedStashItemsRemoved(QList<QListWidgetItem*> item);
    void OnTorchlight2SharedStashFileChanged(QString fileLocation);
    void OnInfiniteStashFolderChanged(QString folderLocation);

private:
    OptionCollection* mOptions;
    SharedStashListWidget* mTorchlight2SharedStashItemsListWidget;
    InfiniteStashItemsListWidget* mInfiniteStashItemsListWidget;
    QComboBox* mGroupsComboBox;
    QLabel* mNumberOfItemsInSharedStashLabel;
    GroupsTable* mGroupsTable;

    SettingsTabPage* mSettingsTabPage;

    Torchlight2Stash* mTorchlight2Stash;

    //Helps us turn off manipulating the shared stash file and sqlite data
    //when we're loading data in from either of them
    bool mIsLoading;

    void FillGroupsComboBox();
    void FillSharedStashList(QString fileLocation);

    
};

#endif // MAINTABPAGE_H
