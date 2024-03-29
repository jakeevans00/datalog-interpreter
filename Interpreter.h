#include "Database.h"
#include <algorithm>
#include <map>
#pragma once

class Interpreter {
    private:
        Database database;
        DatalogProgram datalogProgram;

    public:
        Interpreter(const DatalogProgram& dp) : datalogProgram(dp) {}

        void createDatabase() {
            addRelations();
            addFacts();
            evaluateRules();
        }

        void interpretQueries() {
            cout << "Query Evaluation" << endl;
            for (Predicate query : datalogProgram.getQueries()) {
                Relation relationCopy = database.getRelationCopy(query.getId());
                
                vector<string> parameters = query.getParameters();
                vector<int> columnIndexes;
                vector<string> newColumnNames;
            
                for (unsigned int i = 0; i < parameters.size(); ++i) {
                    string value = parameters.at(i);

                    if (value.at(0) == '\'') {
                        relationCopy = relationCopy.select(i, value);
                    } else {
                        auto it = std::find(newColumnNames.begin(), newColumnNames.end(), value);

                        if (it == newColumnNames.end()) { // only add if not already in newColumnNames
                            columnIndexes.push_back(i);
                            newColumnNames.push_back(value);
                        } else {
                            int j = i + 1;
                            for (int k = 0; k < newColumnNames.size(); ++k) {
                                if (newColumnNames.at(k) == value) {
                                    j = columnIndexes.at(k); // stored index of column name's first occurence
                                    break;
                                }
                            }
                            relationCopy = relationCopy.select(i, j);
                        }
                    }
                }


                Relation projected = relationCopy.project(columnIndexes);
                Relation renamed = projected.rename(newColumnNames);
         
                cout << query.toString() << "? ";
                if (relationCopy.getTuples().size() > 0) {
                    cout << "Yes(" << relationCopy.getTuples().size() << ")" << endl;
                    if (newColumnNames.size() > 0) {
                        cout << renamed.toString();
                    }
                } else {
                    cout << "No" << endl;
                }
            }
        }

        Database getDatabase() {
            return database;
        }

        void addRelations() {
            for (Predicate p : datalogProgram.getSchemes()) {
                Scheme scheme(p.getParameters());
                Relation relation(p.getId(), scheme);
                database.addRelation(relation);
            }
        }

        void addFacts() {
            for (Predicate p : datalogProgram.getFacts()) {
                Tuple tuple(p.getParameters());
                database.addTuple(p.getId(), tuple);
            }
        }

        void evaluateRules() {
            cout << "Rule Evaluation" << endl;
            bool changed = true;
            int passes = 0;
            set<Tuple> newTuples;

            while (changed) {
                changed = false;
                passes++;
                newTuples.clear();

                for (Rule rule : datalogProgram.getRules()) {
                    cout << rule.toString() << endl; 
                    vector<Relation> resultingRelations;

                    for (Predicate bodyPredicate : rule.getBodyPredicates()) {
                        Relation result = interpretPredicate(bodyPredicate);
                        //cout << result.toString() << endl;
                        resultingRelations.push_back(result);
                    }

                    //cout << "resultingRelations size: " << resultingRelations.size() << endl;
                    Relation joined = resultingRelations.at(0);  
                    for (unsigned int i = 1; i < resultingRelations.size(); ++i) {
                        joined = joined.join(resultingRelations.at(i));
                    }

                    //cout << "joined: " << joined.toString() << endl;
                    Relation projected = joined.project_2(rule.getHeadPredicate().getParameters());
                    //cout << "projected: " << projected.toString() << endl;
                    Relation renamed = projected.rename(rule.getHeadPredicate().getParameters());
                    //cout << "renamed: " << renamed.toString() << endl;

                    Relation& headRelation = database.getRelation(rule.getHeadPredicate().getId());
                    
                    newTuples = headRelation.unite(renamed);

                    if (newTuples.size() > 0 ) {
                        changed = true;
                        for (auto& tuple : newTuples) {
                            cout << "  " << tuple.toString(headRelation.getScheme()) << endl;
                        }
                    }
                }   
                
            }
            cout << endl << "Schemes populated after " << passes << " passes through the Rules." << endl << endl;
        }
        

        Relation interpretPredicate(Predicate predicate) {
            Relation relationCopy = database.getRelationCopy(predicate.getId());

            vector<string> parameters = predicate.getParameters();
            vector<int> columnIndexes;
            vector<string> newColumnNames;
        
            for (unsigned int i = 0; i < parameters.size(); ++i) {
                string value = parameters.at(i);

                if (value.at(0) == '\'') {
                    relationCopy = relationCopy.select(i, value);
                } else {
                    auto it = std::find(newColumnNames.begin(), newColumnNames.end(), value);

                    if (it == newColumnNames.end()) { // only add if not already in newColumnNames
                        columnIndexes.push_back(i);
                        newColumnNames.push_back(value);
                    } else {
                        int j = i + 1;
                        for (int k = 0; k < newColumnNames.size(); ++k) {
                            if (newColumnNames.at(k) == value) {
                                j = columnIndexes.at(k); // stored index of column name's first occurence
                                break;
                            }
                        }
                        relationCopy = relationCopy.select(i, j);
                    }
                }
            }


            Relation projected = relationCopy.project(columnIndexes);
            Relation renamed = projected.rename(newColumnNames);

            return renamed;
        }
        
};