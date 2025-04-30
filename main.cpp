#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <cmath>
#include <cstdlib> 
#include <chrono>
#include <unordered_set>

using namespace std;

// Oblicza długość największego sufiksu s1, który jest prefiksem s2
int computeOverlap(const string &s1, const string &s2) {
    int maxLen = min(s1.size(), s2.size());
    for (int len = maxLen; len > 0; --len) {
        if (s1.compare(s1.size() - len, len, s2, 0, len) == 0)
            return len;
    }
    return 0;
}

// Składa sekwencję na podstawie porządku indeksów (order)
// Złączenie odbywa się przez nakładanie fragmentów wg obliczonych overlapów
string assembleSequence(const vector<int> &order, const vector<string>& S, const vector<vector<int>> &overlap) {
    if(order.empty()) return "";
    string seq = S[order[0]];
    for (size_t i = 1; i < order.size(); i++){
        int ov = overlap[order[i-1]][order[i]];
        seq += S[order[i]].substr(ov);
    }
    return seq;
}

// Oblicza długość złożonej sekwencji (bez tworzenia stringa)
int getSequenceLength(const vector<int> &order, const vector<vector<int>> &overlap, int wordLength) {
    if(order.empty()) return 0;
    int length = wordLength;
    for (size_t i = 1; i < order.size(); i++){
        int ov = overlap[order[i-1]][order[i]];
        length += (wordLength - ov);
    }
    return length;
}

// Liczy, ile spośród słów z S występuje jako podciąg w sekwencji
int countCoveredWords(const vector<string>& S, const string &sequence) {
    int count = 0;
    for(const auto &w : S) {
        if(sequence.find(w) != string::npos)
            count++;
    }
    return count;
}

/*
int countCoveredWords(const vector<string>& S, const string &sequence) {
    unordered_set<string> uniqueWords(S.begin(), S.end());  // robimy zbiór unikalnych słów
    int count = 0;
    for(const auto &w : uniqueWords) {
        if(sequence.find(w) != string::npos)
            count++;
    }
    return count;
}
*/

// Rozwiązanie zachłanne – buduje sekwencję zaczynając od startIndex
// Zwraca porządek indeksów, według którego dokonywane jest łączenie
vector<int> greedyPathOrder(const vector<string>& S, const vector<vector<int>> &overlap, int startIndex, int maxLen) {
    int m = S.size();
    int l = S[0].size();
    vector<bool> used(m, false);
    vector<int> order;
    order.push_back(startIndex);
    used[startIndex] = true;
    int currentLen = l;
    int current = startIndex;
    while(true) {
        int best = -1;
        int bestOv = -1;
        for(int i = 0; i < m; i++){
            if(used[i] || i == current) continue;
            int ov = overlap[current][i];
            int newLen = currentLen + (l - ov);
            if(newLen <= maxLen && ov > bestOv) {
                bestOv = ov;
                best = i;
            }
        }
        if(best == -1) break;
        order.push_back(best);
        currentLen += (l - bestOv);
        used[best] = true;
        current = best;
    }
    return order;
}

int main(int argc, char* argv[]) {
    string filename = argv[1];
    int MAX_LEN = atoi(argv[2]);              // Maksymalna długość złożonej sekwencji
    const int NUM_GREEDY_RESTARTS = 100;    // Liczba prób generowania początkowych rozwiązań zachłannych
    const int SA_ITER = 100000;             // Liczba iteracji symulowanego wyżarzania

    ifstream file(filename);
    if(!file) {
        cerr << "Nie można otworzyć pliku!\n";
        return 1;
    }

    vector<string> S;
    string line;
    while(getline(file, line)){
        if(!line.empty())
            S.push_back(line);
    }
    file.close();

    if(S.empty()){
        cerr << "Plik jest pusty!\n";
        return 1;
    }

    int m = S.size();
    int l = S[0].size();
    auto start = chrono::high_resolution_clock::now();
    // Obliczanie macierzy overlapów
    vector<vector<int>> overlap(m, vector<int>(m, 0));
    for(int i = 0; i < m; i++){
        for(int j = 0; j < m; j++){
            if(i != j)
                overlap[i][j] = computeOverlap(S[i], S[j]);
        }
    }
    // Generowanie początkowych rozwiązań za pomocą metody zachłannej
    random_device rd;
    mt19937 rng(rd());
    vector<int> indices(m);
    for (int i = 0; i < m; ++i) indices[i] = i;
    shuffle(indices.begin(), indices.end(), rng);

    vector<int> bestOrder;
    int bestScore = -1;
    string bestSeq;
    for (int i = 0; i < min(NUM_GREEDY_RESTARTS, m); i++){
        int start = indices[i];
        vector<int> order = greedyPathOrder(S, overlap, start, MAX_LEN);
        string seq = assembleSequence(order, S, overlap);
        int score = countCoveredWords(S, seq);
        if(score > bestScore) {
            bestScore = score;
            bestOrder = order;
            bestSeq = seq;
        }
    }
    //cout << bestScore << endl;
    // Ulepszanie rozwiązania metodą symulowanego wyżarzania
    vector<int> currentOrder = bestOrder;
    int currentScore = bestScore;
    string currentSeq = bestSeq;
    int currentLen = getSequenceLength(currentOrder, overlap, l);

    double T = 1.0;          // temperatura początkowa
    double T_min = 1e-4;       // minimalna temperatura
    double alpha = 0.99995;    // współczynnik chłodzenia

    uniform_real_distribution<double> uni(0.0, 1.0);

    for (int iter = 0; iter < SA_ITER; iter++){
        vector<int> newOrder = currentOrder;
        // Losujemy rodzaj ruchu: 0 - zamiana, 1 - usunięcie, 2 - wstawienie
        int moveType = rng() % 3;
        if(moveType == 0 && newOrder.size() >= 2) {
            // Zamiana dwóch losowych elementów
            int i = rng() % newOrder.size();
            int j = rng() % newOrder.size();
            while(j == i) j = rng() % newOrder.size();
            swap(newOrder[i], newOrder[j]);
        } else if(moveType == 1 && newOrder.size() >= 2) {
            // Usunięcie losowego elementu
            int i = rng() % newOrder.size();
            newOrder.erase(newOrder.begin() + i);
        } else if(moveType == 2) {
            // Wstawienie nowego elementu, który nie występuje jeszcze w rozwiązaniu
            vector<bool> used(m, false);
            for (int idx : newOrder) {
                used[idx] = true;
            }
            vector<int> notUsed;
            for (int i = 0; i < m; i++){
                if(!used[i])
                    notUsed.push_back(i);
            }
            if(!notUsed.empty()){
                int newElem = notUsed[rng() % notUsed.size()];
                int pos = rng() % (newOrder.size() + 1);
                newOrder.insert(newOrder.begin() + pos, newElem);
            }
        }

        int newLen = getSequenceLength(newOrder, overlap, l);
        if(newLen > MAX_LEN || newOrder.empty()) {
            // Ruch powoduje przekroczenie limitu – odrzucamy
            continue;
        }
        string newSeq = assembleSequence(newOrder, S, overlap);
        int newScore = countCoveredWords(S, newSeq);

        int delta = newScore - currentScore;
        if(delta >= 0 || uni(rng) < exp(delta / T)) {
            // Akceptujemy ruch (lepszy lub losowo gorszy)
            currentOrder = newOrder;
            currentScore = newScore;
            currentSeq = newSeq;
            currentLen = newLen;
            if(currentScore > bestScore) {
                bestScore = currentScore;
                bestOrder = currentOrder;
                bestSeq = currentSeq;
            }
        }
        T = max(T * alpha, T_min);
    }

    auto end = chrono::high_resolution_clock::now(); // KONIEC pomiaru
    chrono::duration<double> elapsed = end - start;
    cout << "\nCzas wykonania: " << elapsed.count() << " sekund\n";
    cout << "=== SIMULATED ANNEALING ===\n";
    cout << "Max dlugosc sekwencji: " << MAX_LEN << "\n";
    cout << "Najlepsza dlugosc sekwencji: " << getSequenceLength(bestOrder, overlap, l) << "\n";
    cout << "Pokrytych slow: " << bestScore << "\n\n";
    cout << "Sekwencja:\n" << bestSeq << "\n";
    // Zapis wyników do pliku
    ofstream resultFile("results.txt", ios::app); // dopisujemy na końcu pliku
    if (resultFile.is_open()) {
        resultFile << filename << " | " << elapsed.count() << " | " << bestScore << "\n";
        resultFile.close();
    } else {
        cerr << "Nie można otworzyć pliku wynikowego!\n";
    }
    return 0;
}
