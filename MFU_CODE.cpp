// MFU (Most Frequently Used) Page Replacement Demo Program

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
using namespace std;

struct Frame {
    int page = -1;     // -1 means the frame is empty
    int count = 0;     // how many times this page has been used while in the frame
    int loadTime = 0;  // step number when the page was loaded (for FIFO tie-breaking)
};

//  HELPER FUNCTIONS

// Greatest common divisor, used to reduce the hit ratio to lowest terms
int gcd(int a, int b) {
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// A valid page is 1 to 3 digits (0 to 999)
bool isValidPage(const string& s) {
    if (s.empty() || s.size() > 3) return false;
    for (char c : s) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// Waits for the user to press ENTER
void waitForEnter() {
    cout << "      [Press ENTER to show the next step...]";
    string dummy;
    getline(cin, dummy);
}

//  INPUT FUNCTIONS

// INPUT 1: number of frames (returns the number entered)
int readFrames() {
    string line;
    int numFrames = 0;
    while (true) {
        cout << "Enter number of page frames: ";
        if (!getline(cin, line)) exit(1);
        stringstream ss(line);
        string extra;
        if ((ss >> numFrames) && numFrames > 0 && !(ss >> extra)) break;
        cout << "  Invalid input. Enter a whole number greater than 0.\n";
    }
    return numFrames;
}

// INPUT 2: reference string (returns the list of pages entered)
vector<int> readReferenceString() {
    string line;
    vector<int> pages;
    while (true) {
        cout << "\nEnter the reference string.\n";
        cout << "Each page must be 1 to 3 digits, separated by spaces.\n";
        cout << "Example: 7 0 1 2 0 3 0 4 2 3 0 3 2\n";
        cout << "Reference string: ";
        if (!getline(cin, line)) exit(1);

        pages.clear();
        stringstream ss(line);
        string token;
        bool ok = true;
        while (ss >> token) {
            if (!isValidPage(token)) {
                cout << "  Invalid page \"" << token
                     << "\". Use only numbers of 1 to 3 digits.\n";
                ok = false;
                break;
            }
            pages.push_back(stoi(token));
        }
        if (ok && !pages.empty()) break;
        if (ok) cout << "  You did not enter any pages.\n";
    }
    return pages;
}

//  MFU PROCESSING FUNCTIONS

// does the page already in a frame? Returns the frame index, or -1 if not found.
int findHit(const vector<Frame>& frames, int page) {
    int n = frames.size();
    for (int i = 0; i < n; i++) {
        if (frames[i].page == page) return i;
    }
    return -1;
}

// finds an empty frame. Returns its index, or -1 if all frames are full.
int findEmptyFrame(const vector<Frame>& frames) {
    int n = frames.size();
    for (int i = 0; i < n; i++) {
        if (frames[i].page == -1) return i;
    }
    return -1;
}

// DEQUEUE - chooses the victim page to remove (all frames must be full).
// Returns the victim's frame index.
// 'tied' is set to how many pages share the highest counter.
int findVictim(const vector<Frame>& frames, int& tied) {
    int n = frames.size();

    // MFU rule: highest counter. FIFO rule for ties: earliest loadTime.
    int victim = 0;
    for (int i = 1; i < n; i++) {
        if (frames[i].count > frames[victim].count ||
            (frames[i].count == frames[victim].count &&
             frames[i].loadTime < frames[victim].loadTime)) {
            victim = i;
        }
    }

    // Count how many frames share the highest counter
    tied = 0;
    for (int i = 0; i < n; i++) {
        if (frames[i].count == frames[victim].count) tied++;
    }
    return victim;
}

// ENQUEUE - places a page into a frame (the old page, if any, is overwritten)
void loadPage(Frame& frame, int page, int step) {
    frame.page = page;
    frame.count = 1;        // a newly loaded page starts at 1
    frame.loadTime = step;  // remembered for FIFO tie-breaking
}
//  DISPLAY FUNCTIONS

void showInstructions() {
    string line;
    cout << "\n-----------------------------------------------------\n";
    cout << "Frames are shown as  page(counter).  '-' means empty.\n";
    cout << "H = Hit.   Fault = page had to be loaded.\n";
    cout << "The steps are shown ONE AT A TIME.\n";
    cout << "Press ENTER after each step to continue.\n";
    cout << "-----------------------------------------------------\n";
    cout << "Press ENTER to start...";
    getline(cin, line);
}

void showTableHeader(int numFrames) {
    cout << "\n" << left << setw(6) << "Step" << setw(6) << "Page";
    for (int i = 0; i < numFrames; i++) {
        cout << setw(10) << ("F" + to_string(i + 1));
    }
    cout << "Result\n";
    cout << string(12 + 10 * numFrames + 8, '-') << "\n";
}

// Prints one step: the page, every frame as page(counter), H or Fault, and a note
void showStep(int step, int page, const vector<Frame>& frames, bool hit, const string& note) {
    int n = frames.size();
    cout << left << setw(6) << (step + 1) << setw(6) << page;
    for (int i = 0; i < n; i++) {
        string cell;
        if (frames[i].page == -1)
            cell = "-";
        else
            cell = to_string(frames[i].page) + "(" + to_string(frames[i].count) + ")";
        cout << setw(10) << cell;
    }
    cout << (hit ? "H" : "Fault") << "\n";
    cout << "      -> " << note << "\n";
}

// Prints totals and the hit ratio (original and lowest terms)
void showSummary(int hits, int totalRefs) {
    int faults = totalRefs - hits;
    int g = gcd(hits, totalRefs);   // gcd(0, n) = n, so 0 hits becomes 0/1

    cout << "\n ----- SUMMARY -----\n";
    cout << "Total page references : " << totalRefs << "\n";
    cout << "Total hits (H)        : " << hits << "\n";
    cout << "Total faults          : " << faults << "\n";
    cout << "\nHit ratio (original)       : " << hits << "/" << totalRefs << "\n";
    cout << "Hit ratio (lowest terms)   : " << hits / g << "/" << totalRefs / g << "\n";
    cout << "Hit ratio (percentage)     : " << fixed << setprecision(2)
         << (100.0 * hits / totalRefs) << "%\n";
}
//  MAIN

int main() {
    cout << "  ----- MFU (Most Frequently Used) Page Replacement -----\n";

    // INPUT
    int numFrames = readFrames();
    vector<int> pages = readReferenceString();

    int totalRefs = static_cast<int>(pages.size());
    vector<Frame> frames(numFrames);
    int hits = 0;

    // INSTRUCTIONS AND TABLE HEADER
    showInstructions();
    showTableHeader(numFrames);

    // PROCESSING: one page reference at a time
    for (int step = 0; step < totalRefs; step++) {
        int current = pages[step];
        bool hit = false;
        string note;

        int hitIndex = findHit(frames, current);

        if (hitIndex != -1) {
            // HIT: the page is already in memory
            frames[hitIndex].count++;
            hit = true;
            hits++;
            note = "Page " + to_string(current) + " is already in memory. "
                   "Its counter is now " + to_string(frames[hitIndex].count) + ".";
            
        } else {
            // FAULT: the page must be loaded
            int target = findEmptyFrame(frames);

            if (target != -1) {
                note = "Page " + to_string(current) + " loaded into empty frame F"
                       + to_string(target + 1) + ".";
            } else {
                // Frames are full: DEQUEUE a victim (highest counter, FIFO on ties)
                int tied = 0;
                target = findVictim(frames, tied);

                if (tied > 1) {
                    note = "Frames full. " + to_string(tied) + " pages tied for the highest counter ("
                           + to_string(frames[target].count) + "). FIFO is used to break the tie: "
                           + "Page " + to_string(frames[target].page)
                           + " was loaded first and is REMOVED. Page " + to_string(current)
                           + " takes its place in F" + to_string(target + 1) + ".";
                } else {
                    note = "Frames full. Page " + to_string(frames[target].page)
                           + " has the highest counter (" + to_string(frames[target].count)
                           + ") and is REMOVED. Page " + to_string(current)
                           + " takes its place in F" + to_string(target + 1) + ".";
                }
            }
            // ENQUEUE the new page into the chosen frame
            loadPage(frames[target], current, step);
        }

        // DISPLAY this step, then wait for the user
        showStep(step, current, frames, hit, note);

        if (step < totalRefs - 1) {
            waitForEnter();
            cout << "\n";
        }
    }

    // SUMMARY section
    showSummary(hits, totalRefs);

    return 0;
}             





//COMMENTS/GUIDES USED IN THE CODE

// Rules used by this program:
//   - A page already in a frame is a HIT (its use counter goes up by 1).
//   - A page not in a frame is a FAULT. If there is an empty frame it is used.
//   - If all frames are full, the page with the HIGHEST use counter is removed.
//   - If two or more pages tie for the highest counter, FIFO breaks the tie:
//     the page that was loaded earliest is removed.
//   - A newly loaded page starts with a counter of 1.

// Terms used in the presentation:
//   ENQUEUE = loading a page into a frame
//   DEQUEUE = removing the victim page from a frame
//
// Function connections (values passed):
//   main()
//    |- readFrames()                   -> returns numFrames
//    |- readReferenceString()          -> returns pages   (uses isValidPage)
//    |- showInstructions()
//    |- showTableHeader(numFrames)
//    |- for each page:
//    |    |- findHit(frames, page)     -> returns frame index or -1
//    |    |- findEmptyFrame(frames)    -> returns frame index or -1
//    |    |- findVictim(frames, tied)  -> returns frame index (DEQUEUE), sets tied
//    |    |- loadPage(frame, page, step)  (ENQUEUE)
//    |    |- showStep(...)
//    |    '- waitForEnter()
//    '- showSummary(hits, totalRefs)   -> uses gcd(hits, totalRefs)

