#include "maintabpageform.h"
#include "ui_maintabpageform.h"

#include <torchlight2stashconverter.h>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>

MainTabPageForm::MainTabPageForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTabPageForm)
{
    ui->setupUi(this);

    mTorchlight2Stash = NULL;
    mStashItemsTable = NULL;

    mInfiniteStashTreeViewModel = NULL;

    mIsLoading = false;

    connect(ui->Torchlight2SharedStashListWidget,
            SIGNAL(itemAdded(QListWidgetItem*)), this,
            SLOT(OnTorchlight2SharedStashItemAdded(QListWidgetItem*)));

    connect(ui->Torchlight2SharedStashListWidget,
            SIGNAL(itemsAboutToBeRemoved(QList<QListWidgetItem*>)), this,
            SLOT(OnTorchlight2SharedStashItemsRemoved(QList<QListWidgetItem*>)));


//    connect(ui->InfiniteStashItemsTreeView,
//            SIGNAL(itemDropped(QDropEvent*)), this,
//            SLOT(OnItemDroppedOnInfiniteStash(QDropEvent*)));

    connect(ui->Torchlight2SharedStashListWidget,
            SIGNAL(dragEntered(QDragEnterEvent*)),
            this,
            SLOT(OnItemDraggedToTorchlight2SharedStash(QDragEnterEvent*)));

    connect(ui->Torchlight2SharedStashListWidget,
            SIGNAL(itemDropped(QDropEvent*)), this,
            SLOT(OnItemDroppedOnTorchlight2SharedStash(QDropEvent*)));

    connect(ui->Torchlight2SharedStashListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(OnTorchlight2StashItemActivated(int)));
}

MainTabPageForm::~MainTabPageForm()
{
    delete ui;
}

void MainTabPageForm::SetInfiniteStashStandardItemModel(InfiniteStashStandardItemModel* inModel)
{
    mInfiniteStashTreeViewModel = inModel;

    if (mInfiniteStashTreeViewModel != NULL)
    {
        ui->InfiniteStashItemsTreeView->setModel(mInfiniteStashTreeViewModel);
    }
}

void MainTabPageForm::Options(OptionCollection *inOptions)
{
    mOptions = inOptions;

    if (mOptions != NULL)
    {
        QString torchlight2StashFile = mOptions->Get(OptionKeys::Torchlight2SharedStashFile);
        QString infiniteStashFolder = mOptions->Get(OptionKeys::StashManagerFolder);

        ui->StashItemDetailsGraphicsView->InfiniteStashLocation(infiniteStashFolder);

        FillSharedStashList(torchlight2StashFile);
    }
}

void MainTabPageForm::FillSharedStashList(QString inTorchlight2SharedStashFile)
{

    mIsLoading = true;

    ui->Torchlight2SharedStashListWidget->clear();

    if (mTorchlight2Stash != NULL)
        delete mTorchlight2Stash;

    QByteArray decryptedBytes;

    QFile stashFile(inTorchlight2SharedStashFile);

    if (stashFile.open(QIODevice::ReadOnly))
    {
        if (Torchlight2StashConverter::DescrambleFile(stashFile.readAll(), decryptedBytes))
        {
            stashFile.close();

            QDir stashManagerFolder(mOptions->Get(OptionKeys::StashManagerFolder));
            QFileInfo path(inTorchlight2SharedStashFile);
            QString absolutePath = stashManagerFolder.absolutePath();
            QString fileName = path.fileName();

            fileName = "decrypted_" + fileName;

            QFile decryptedFile(absolutePath + "/" + fileName);

            if (decryptedFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                decryptedFile.write(decryptedBytes);
                decryptedFile.close();
            }

            mTorchlight2Stash = new Torchlight2Stash(decryptedBytes);

            QString maxNumberOfItems = mOptions->Get(OptionKeys::MaxNumberOfItemsPerStashTab);

            mTorchlight2Stash->MaximumNumberOfItemsPerTab(maxNumberOfItems.toInt());

            QList<Torchlight2Item> itemsInStash = mTorchlight2Stash->StashItems();
    //        QList<QString> keys = itemsInStash.keys();

            for (qint32 i = 0; i < itemsInStash.count(); ++i)
            {
                QString itemName = itemsInStash[i].Name();
                itemName.replace(":", "");
                QFile nextItem(absolutePath + "/" + QString::number(i) + itemName + ".dat");

                if (nextItem.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                    nextItem.write(itemsInStash[i].Bytes());
                    nextItem.close();
                }

                QListWidgetItem* item = new QListWidgetItem(itemsInStash[i].Name());
    //                item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);

                QVariant itemData = QVariant::fromValue(itemsInStash[i].Bytes());
    //            QVariant itemData(*itemsInStash[keys[i]]);

                //first user role is the items data
                item->setData(Qt::UserRole, itemData);

                //second user role is a marker so we know that this is an item from the stash
                item->setData(Qt::UserRole + 1, InfiniteStashStandardItemModel::StashItemType);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
                ui->Torchlight2SharedStashListWidget->addItem(item);
            }
        }
    }

    mIsLoading = false;
}

void MainTabPageForm::OnTorchlight2SharedStashItemAdded(QListWidgetItem* item)
{
    qint32 count = ui->Torchlight2SharedStashListWidget->count();
    ui->SharedStashItemCountLabel->setText(QString::number(count));

    if (!mIsLoading && mStashItemsTable != NULL)
    {
        QVariant itemId = item->data(Qt::UserRole);
        StashItem itemData = mStashItemsTable->GetStashItem(itemId.toLongLong());

        if (mStashItemsTable->DeleteStashItem(itemId.toLongLong()))
        {

        }
    }
}

void MainTabPageForm::OnTorchlight2SharedStashItemsRemoved(QList<QListWidgetItem*> items)
{
    qint32 count = ui->Torchlight2SharedStashListWidget->count() - items.count();

    ui->SharedStashItemCountLabel->setText(QString::number(count));

    for (int i = 0; i < items.length(); ++i)
    {
        if (mStashItemsTable != NULL)
        {
            StashItem itemData;
            itemData.item = items[i]->data(Qt::UserRole).toByteArray();
            itemData.itemId = mStashItemsTable->AddStashItem(itemData);

            if (itemData.itemId > 0)
            {
                items[i]->setData(Qt::UserRole, itemData.itemId);
            }
        }
    }
}

void MainTabPageForm::OnTorchlight2SharedStashFileChanged(QString fileLocation)
{
    FillSharedStashList(fileLocation);
}

void MainTabPageForm::OnMaximumNumberOfItemsPerTabChanged(qint32 newMax)
{
    if (mTorchlight2Stash != NULL)
        mTorchlight2Stash->MaximumNumberOfItemsPerTab(newMax);
}


void MainTabPageForm::OnItemDroppedOnTorchlight2SharedStash(QDropEvent *event)
{
    if (event->source() == ui->InfiniteStashItemsTreeView)
    {
        QListWidgetItem* itemDropped = ui->Torchlight2SharedStashListWidget->GetDroppedItem(event->pos());

        StashItem itemData = mStashItemsTable->GetStashItem(itemDropped->data(Qt::UserRole).toLongLong());

        Torchlight2Item stashItem(itemData.item);
        mTorchlight2Stash->AddItemToStash(stashItem);
        QByteArray outputBytes;


        if (Torchlight2StashConverter::ScrambleFile(mTorchlight2Stash->Bytes(), outputBytes))
        {
            QString stashFilePath = mOptions->Get(OptionKeys::Torchlight2SharedStashFile);
            QFile stashFile(stashFilePath);

            if (stashFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qint64 numBytesWritten = stashFile.write(outputBytes);

                stashFile.close();

                if (numBytesWritten == outputBytes.length())
                {
                    if (mStashItemsTable->DeleteStashItem(itemData.itemId))
                    {
                        itemDropped->setData(Qt::UserRole, itemData.item);
                    }
                }
            }
        }



    }
}


/*
 *  We capture this event in order to prevent groups from being dragged onto the
 *  Torchlight 2 stash list.
 *
 */
void MainTabPageForm::OnItemDraggedToTorchlight2SharedStash(QDragEnterEvent* event)
{
    bool ignoreAction = false;

    if (event->source() == ui->InfiniteStashItemsTreeView)
    {
        QByteArray modelList = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
        QDataStream inputStream(&modelList, QIODevice::ReadOnly);

        while (!inputStream.atEnd())
        {
            int row;
            int col;
            QMap<int, QVariant> roleDataMap;
            inputStream >> row >> col >> roleDataMap;

            QVariant itemType = roleDataMap[Qt::UserRole + 1];

            if (itemType.toInt() == InfiniteStashStandardItemModel::GroupItemType)
            {
                ignoreAction = true;
            }
            else if (itemType.toInt() == InfiniteStashStandardItemModel::StashItemType)
            {
                QVariant itemIdVariant = roleDataMap[Qt::UserRole];

                if (!itemIdVariant.isNull() && itemIdVariant.isValid() &&
                        itemIdVariant.type() == QVariant::LongLong)
                {
                    StashItem itemDetails = mStashItemsTable->GetStashItem(itemIdVariant.toLongLong());
                    Torchlight2Item item(itemDetails.item);

                    if (!mTorchlight2Stash->CanItemBeAdded(item))
                    {
                        ignoreAction = true;
                    }
                }
            }
        }
    }

    if (ignoreAction)
    {
        event->ignore();
        event->setDropAction(Qt::IgnoreAction);
    }
}

void MainTabPageForm::OnTorchlight2StashItemActivated(int row)
{
    QListWidgetItem* widgetItem = ui->Torchlight2SharedStashListWidget->item(row);

    Torchlight2Item item(widgetItem->data(Qt::UserRole).toByteArray());
    ui->StashItemDetailsGraphicsView->SetItemToDisplay(item);

}
