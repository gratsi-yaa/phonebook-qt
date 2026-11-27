#pragma once
#include <QDialog>
#include "Contact.h"

class QCheckBox;
class QDateEdit;
class QLineEdit;
class QTableWidget;

// Dialog to create or edit a contact. Every field is checked with the
// regular expressions of the Validator before the dialog can be accepted.
class ContactDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ContactDialog(QWidget *parent = nullptr, const Contact *initial = nullptr);
    Contact contact() const { return m_result; }

private slots:
    void onAccept();

private:
    void addPhoneRow(PhoneType type, const QString &number);
    QStringList validateAll(); // fills m_result, returns the list of errors

    QLineEdit *m_last;
    QLineEdit *m_first;
    QLineEdit *m_middle;
    QLineEdit *m_address;
    QLineEdit *m_email;
    QCheckBox *m_hasBirth;
    QDateEdit *m_birth;
    QTableWidget *m_phones;
    Contact m_result;
};