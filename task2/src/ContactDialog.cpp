#include "ContactDialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "QtUtil.h"
#include "Validator.h"

namespace
{
    const char *kErrorStyle = "border: 1px solid red;";
}

ContactDialog::ContactDialog(QWidget *parent, const Contact *initial) : QDialog(parent)
{
    setWindowTitle(initial ? tr("Edit contact") : tr("New contact"));
    setMinimumWidth(560);

    m_last = new QLineEdit(this);
    m_first = new QLineEdit(this);
    m_middle = new QLineEdit(this);
    m_address = new QLineEdit(this);
    m_email = new QLineEdit(this);

    m_hasBirth = new QCheckBox(tr("Specify birth date"), this);
    m_birth = new QDateEdit(this);
    m_birth->setCalendarPopup(true); // calendar widget drop-down
    m_birth->setDisplayFormat("dd.MM.yyyy");
    m_birth->setMaximumDate(QDate::currentDate().addDays(-1)); // strictly before today
    m_birth->setDate(QDate(2000, 1, 1));
    m_birth->setEnabled(false);
    connect(m_hasBirth, &QCheckBox::toggled, m_birth, &QWidget::setEnabled);

    auto *birthRow = new QHBoxLayout;
    birthRow->addWidget(m_hasBirth);
    birthRow->addWidget(m_birth);
    birthRow->addStretch();

    m_phones = new QTableWidget(0, 2, this);
    m_phones->setHorizontalHeaderLabels({tr("Type"), tr("Number")});
    m_phones->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_phones->verticalHeader()->hide();
    m_phones->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_phones->setMinimumHeight(130);

    auto *addPhone = new QPushButton(tr("Add number"), this);
    auto *removePhone = new QPushButton(tr("Remove selected"), this);
    connect(addPhone, &QPushButton::clicked, this, [this]
            { addPhoneRow(PhoneType::Work, QString()); });
    connect(removePhone, &QPushButton::clicked, this, [this]
            {
        int row = m_phones->currentRow();
        if (row >= 0) m_phones->removeRow(row); });
    auto *phoneButtons = new QHBoxLayout;
    phoneButtons->addWidget(addPhone);
    phoneButtons->addWidget(removePhone);
    phoneButtons->addStretch();

    auto *form = new QFormLayout;
    form->addRow(tr("Last name *"), m_last);
    form->addRow(tr("First name *"), m_first);
    form->addRow(tr("Middle name"), m_middle);
    form->addRow(tr("Address"), m_address);
    form->addRow(tr("E-mail *"), m_email);
    form->addRow(tr("Birth date"), birthRow);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &ContactDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Fields marked * are required (at least one phone number)."), this));
    layout->addLayout(form);
    layout->addWidget(new QLabel(tr("Phone numbers *"), this));
    layout->addWidget(m_phones);
    layout->addLayout(phoneButtons);
    layout->addWidget(buttons);

    if (initial)
    {
        m_result.id = initial->id;
        m_last->setText(fromStd(initial->lastName));
        m_first->setText(fromStd(initial->firstName));
        m_middle->setText(fromStd(initial->middleName));
        m_address->setText(fromStd(initial->address));
        m_email->setText(fromStd(initial->email));
        if (initial->birthDate)
        {
            m_hasBirth->setChecked(true);
            m_birth->setDate(QDate(initial->birthDate->year, initial->birthDate->month,
                                   initial->birthDate->day));
        }
        for (const PhoneNumber &p : initial->phones)
            addPhoneRow(p.type, fromStd(p.number));
    }
    else
    {
        addPhoneRow(PhoneType::Work, QString());
    }
}

void ContactDialog::addPhoneRow(PhoneType type, const QString &number)
{
    int row = m_phones->rowCount();
    m_phones->insertRow(row);
    auto *combo = new QComboBox(m_phones);
    combo->addItems({tr("Work"), tr("Home"), tr("Service")});
    combo->setCurrentIndex(static_cast<int>(type)); // enum order: Work, Home, Service
    m_phones->setCellWidget(row, 0, combo);
    m_phones->setItem(row, 1, new QTableWidgetItem(number));
}

QStringList ContactDialog::validateAll()
{
    QStringList errors;
    std::string v;

    auto check = [&](QLineEdit *edit, bool required,
                     bool (*norm)(const std::string &, std::string &),
                     const QString &message, std::string &target)
    {
        edit->setStyleSheet(QString());
        std::string raw = toStd(edit->text());
        if (!required && Validator::trim(raw).empty())
        {
            target.clear();
            edit->clear();
            return;
        }
        if (norm(raw, v))
        {
            target = v;
            edit->setText(fromStd(v)); // show the cleaned value
        }
        else
        {
            edit->setStyleSheet(kErrorStyle);
            errors << message;
        }
    };

    const QString nameRule = tr("letters, digits, '-' and spaces; starts with a letter; no '-' at the start or end");
    check(m_last, true, &Validator::normalizeName, tr("Last name: ") + nameRule, m_result.lastName);
    check(m_first, true, &Validator::normalizeName, tr("First name: ") + nameRule, m_result.firstName);
    check(m_middle, false, &Validator::normalizeName, tr("Middle name: ") + nameRule, m_result.middleName);
    check(m_address, false, &Validator::normalizeAddress,
          tr("Address: letters, digits, spaces and . , / # ( ) - only"), m_result.address);
    check(m_email, true, &Validator::normalizeEmail,
          tr("E-mail: expected user@domain (Latin letters and digits)"), m_result.email);

    m_result.birthDate.reset();
    if (m_hasBirth->isChecked())
    {
        QDate d = m_birth->date();
        Date date{d.year(), d.month(), d.day()};
        if (d.isValid() && date < today())
            m_result.birthDate = date;
        else
            errors << tr("Birth date: must be a real date before today");
    }

    m_result.phones.clear();
    bool phoneError = false;
    for (int row = 0; row < m_phones->rowCount(); ++row)
    {
        auto *combo = qobject_cast<QComboBox *>(m_phones->cellWidget(row, 0));
        QTableWidgetItem *item = m_phones->item(row, 1);
        std::string number;
        if (combo && item && Validator::normalizePhone(toStd(item->text()), number))
        {
            item->setBackground(QBrush());
            item->setText(fromStd(number));
            m_result.phones.push_back({static_cast<PhoneType>(combo->currentIndex()), number});
        }
        else
        {
            if (item)
                item->setBackground(QColor(255, 200, 200));
            phoneError = true;
        }
    }
    if (phoneError)
        errors << tr("Phone: use +78121234567, 88121234567, +7(812)1234567, 8(812)1234567, "
                     "+7(812)123-45-67 or 8(812)123-45-67");
    else if (m_phones->rowCount() == 0)
        errors << tr("At least one phone number is required");

    return errors;
}

void ContactDialog::onAccept()
{
    QStringList errors = validateAll();
    if (!errors.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid data"), errors.join("\n\n"));
        return;
    }
    accept();
}