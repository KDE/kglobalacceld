#include <QKeySequence>

#include "kglobalshortcutinfo_p.h"
#include "sequencehelpers_p.h"

#include <QList>

namespace Utils
{
QKeySequence reverseKey(const QKeySequence &key)
{
    int k[maxSequenceLength] = {0, 0, 0, 0};
    int count = key.count();
    for (int i = 0; i < count; i++) {
        k[count - i - 1] = key[i].toCombined();
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

QKeySequence cropKey(const QKeySequence &key, int count)
{
    if (count < 1) {
        return key;
    }

    // Key is shorter than count we want to cut off
    if (key.count() < count) {
        return QKeySequence();
    }

    int k[maxSequenceLength] = {0, 0, 0, 0};
    // cut from beginning
    for (int i = count; i < key.count(); i++) {
        k[i - count] = key[i].toCombined();
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

bool contains(const QKeySequence &key, const QKeySequence &other)
{
    int minLength = std::min(key.count(), other.count());

    // There's an empty key, assume it matches nothing
    if (!minLength) {
        return false;
    }

    bool ret = false;
    for (int i = 0; i <= other.count() - minLength; i++) {
        QKeySequence otherCropped = cropKey(other, i);
        if (key.matches(otherCropped) == QKeySequence::PartialMatch || reverseKey(key).matches(reverseKey(otherCropped)) == QKeySequence::PartialMatch) {
            ret = true;
            break;
        }
    }

    return ret;
}

Qt::KeyboardModifiers keyToModifier(int key)
{
    switch (key) {
    case Qt::Key_Meta:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        // Qt doesn't properly recognize Super_L/Super_R as MetaModifier
        return Qt::MetaModifier;
    case Qt::Key_Shift:
        return Qt::ShiftModifier;
    case Qt::Key_Control:
        return Qt::ControlModifier;
    case Qt::Key_Alt:
        return Qt::AltModifier;
    default:
        return Qt::NoModifier;
    }
}

bool matchSequences(const QKeySequence &key, const QSet<QKeySequence> &keys)
{
    // Since we're testing sequences, we need to check for all possible matches
    // between existing and new sequences.

    // Let's assume we have (Alt+B, Alt+F, Alt+G) assigned. Examples of bad shortcuts are:
    // 1) Exact matching: (Alt+B, Alt+F, Alt+G)
    // 2) Sequence shadowing: (Alt+B, Alt+F)
    // 3) Sequence being shadowed: (Alt+B, Alt+F, Alt+G, <any key>)
    // 4) Shadowing at the end: (Alt+F, Alt+G)
    // 5) Being shadowed from the end: (<any key>, Alt+B, Alt+F, Alt+G)

    for (const QKeySequence &otherKey : keys) {
        if (otherKey.isEmpty()) {
            continue;
        }
        if (key.matches(otherKey) == QKeySequence::ExactMatch || contains(key, otherKey) || contains(otherKey, key)) {
            return true;
        }
    }
    return false;
}

static QKeyCombination normalizeKey(QKeyCombination keyCombination)
{
    QKeyCombination normalizedCombination;
    switch (keyCombination.key()) {
    case Qt::Key_Shift:
        normalizedCombination = Qt::Key(0) | (keyCombination.keyboardModifiers() | Qt::ShiftModifier);
        break;
    case Qt::Key_Control:
        normalizedCombination = Qt::Key(0) | (keyCombination.keyboardModifiers() | Qt::ControlModifier);
        break;
    case Qt::Key_Alt:
        normalizedCombination = Qt::Key(0) | (keyCombination.keyboardModifiers() | Qt::AltModifier);
        break;
    case Qt::Key_Meta:
        normalizedCombination = Qt::Key(0) | (keyCombination.keyboardModifiers() | Qt::MetaModifier);
        break;
    default:
        normalizedCombination = keyCombination;
        break;
    }

    // QKeySequence doesn't support modifier-only shortcuts so we replace the last modifier in the
    // key sequence with a key code. The modifiers are always ordered as Meta + Ctrl + Alt + Shift.
    // See https://qt-project.atlassian.net/browse/QTBUG-132435.
    if (!normalizedCombination.key()) {
        const Qt::KeyboardModifiers modifiers = normalizedCombination.keyboardModifiers();

        if (modifiers & Qt::ShiftModifier) {
            normalizedCombination = Qt::Key_Shift | (modifiers & ~Qt::ShiftModifier);
        } else if (modifiers & Qt::AltModifier) {
            normalizedCombination = Qt::Key_Alt | (modifiers & ~Qt::AltModifier);
        } else if (modifiers & Qt::ControlModifier) {
            normalizedCombination = Qt::Key_Control | (modifiers & ~Qt::ControlModifier);
        } else if (modifiers & Qt::MetaModifier) {
            normalizedCombination = Qt::Key_Meta | (modifiers & ~Qt::MetaModifier);
        }
    }

    return normalizedCombination;
}

QKeySequence normalizeSequence(const QKeySequence &key)
{
    // Qt triggers both shortcuts that include Shift+Backtab and Shift+Tab
    // when user presses Shift+Tab. Make no difference here.
    int k[maxSequenceLength] = {0, 0, 0, 0};
    for (int i = 0; i < key.count(); i++) {
        // Qt triggers both shortcuts that include Shift+Backtab and Shift+Tab
        // when user presses Shift+Tab. Make no difference here.
        const int keySym = key[i].toCombined() & ~Qt::KeyboardModifierMask;
        const int keyMod = key[i].toCombined() & Qt::KeyboardModifierMask;
        if (keySym == Qt::Key_Backtab) {
            k[i] = keyMod | Qt::ShiftModifier | Qt::Key_Tab;
        } else {
            k[i] = normalizeKey(key[i]).toCombined();
        }
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

QSet<QKeySequence> normalizeSequences(const QSet<QKeySequence> &keys)
{
    QSet<QKeySequence> ret;
    ret.reserve(keys.size());
    for (const QKeySequence &key : keys) {
        ret.insert(normalizeSequence(key));
    }
    return ret;
}

} // namespace Utils
