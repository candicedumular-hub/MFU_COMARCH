// MFU (Most Frequently Used) Page Replacement Demo
// Rules used by this program:
//   - A page already in a frame is a HIT (its use counter goes up by 1).
//   - A page not in a frame is a FAULT. If there is an empty frame it is used.
//   - If all frames are full, the page with the HIGHEST use counter is removed.
//   - If two or more pages tie for the highest counter, FIFO breaks the tie:
//     the page that was loaded earliest is removed.
//   - A newly loaded page starts with a counter of 1.

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
using namespace std;

struct Frame {
    int page = -1;     // -1 means the frame is empty
    int count = 0;     // how many times this page has been used while in the frame
    int loadTime = 0;  // step number when the page was loaded (for tie-breaking)
};

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

int main() {
    string line;


    cout << "  ----- MFU (Most Frequently Used) Page Replacement -----\n";


    // INPUT 1: number of frames
    int numFrames = 0;
    while (true) {
        cout << "Enter number of page frames: ";
        if (!getline(cin, line)) return 1;
        stringstream ss(line);
        string extra;
        if ((ss >> numFrames) && numFrames > 0 && !(ss >> extra)) break;
        cout << "  Invalid input. Enter a whole number greater than 0.\n";
    }

    // INPUT 2: reference string 
    vector<int> pages;
    while (true) {
        cout << "\nEnter the reference string.\n";
        cout << "Each page must be 1 to 3 digits, separated by spaces.\n";
        cout << "Example: 7 0 1 2 0 3 0 4 2 3 0 3 2\n";
        cout << "Reference string: ";
        if (!getline(cin, line)) return 1;

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

    int totalRefs = static_cast<int>(pages.size());
    vector<Frame> frames(numFrames);
    int hits = 0;

    // INSTRUCTIONS
    cout << "\n-----------------------------------------------------\n";
    cout << "Frames are shown as  page(counter).  '-' means empty.\n";
    cout << "H = Hit.   Fault = page had to be loaded.\n";
    cout << "The steps are shown ONE AT A TIME.\n";
    cout << "Press ENTER after each step to continue.\n";
    cout << "-----------------------------------------------------\n";
    cout << "Press ENTER to start...";
    getline(cin, line);

    // TABLE HEADER 
    cout << "\n" << left << setw(6) << "Step" << setw(6) << "Page";
    for (int i = 0; i < numFrames; i++) {
        cout << setw(10) << ("F" + to_string(i + 1));
    }
    cout << "Result\n";
    cout << string(12 + 10 * numFrames + 8, '-') << "\n";

    // SIMULATION 
    for (int step = 0; step < totalRefs; step++) {
        int current = pages[step];
        bool hit = false;
        string note;

        // Is the page already in a frame?
        for (int i = 0; i < numFrames; i++) {
            if (frames[i].page == current) {
                frames[i].count++;
                hit = true;
                hits++;
                note = "Page " + to_string(current) + " is already in memory. "
                       "Its counter is now " + to_string(frames[i].count) + ".";
                break;
            }
        }

        // Page fault
        if (!hit) {
            int target = -1;

            // Use an empty frame if there is one
            for (int i = 0; i < numFrames; i++) {
                if (frames[i].page == -1) {
                    target = i;
                    break;
                }
            }

            if (target != -1) {
                note = "Page " + to_string(current) + " loaded into empty frame F"
                       + to_string(target + 1) + ".";
            } else {
                // All frames full: choose the highest counter (tie -> earliest loaded)
                target = 0;
                for (int i = 1; i < numFrames; i++) {
                    if (frames[i].count > frames[target].count ||
                        (frames[i].count == frames[target].count &&
                         frames[i].loadTime < frames[target].loadTime)) {
                        target = i;
                    }
                }
                // How many frames share the highest counter?
                int tied = 0;
                for (int i = 0; i < numFrames; i++) {
                    if (frames[i].count == frames[target].count) tied++;
                }

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

            frames[target].page = current;
            frames[target].count = 1;
            frames[target].loadTime = step;
        }

        // Print this step
        cout << left << setw(6) << (step + 1) << setw(6) << current;
        for (int i = 0; i < numFrames; i++) {
            string cell;
            if (frames[i].page == -1)
                cell = "-";
            else
                cell = to_string(frames[i].page) + "(" + to_string(frames[i].count) + ")";
            cout << setw(10) << cell;
        }
        cout << (hit ? "H" : "Fault") << "\n";
        cout << "      -> " << note << "\n";

        if (step < totalRefs - 1) {
            waitForEnter();
            cout << "\n";
        }
    }

    // ---------- SUMMARY ----------
    int faults = totalRefs - hits;
    int g = gcd(hits, totalRefs);   // gcd(0, n) = n, so 0 hits becomes 0/1
    int reducedHits = hits / g;
    int reducedTotal = totalRefs / g;

    
    cout << " ----- SUMMARY -----\n";
  
    cout << "Total page references : " << totalRefs << "\n";
    cout << "Total hits (H)        : " << hits << "\n";
    cout << "Total faults          : " << faults << "\n";
    cout << "\nHit ratio (original)       : " << hits << "/" << totalRefs << "\n";
    cout << "Hit ratio (lowest terms)   : " << reducedHits << "/" << reducedTotal << "\n";
    cout << "Hit ratio (percentage)     : " << fixed << setprecision(2)
         << (100.0 * hits / totalRefs) << "%\n";
    cout << "=====================================================\n";

    return 0;
}
