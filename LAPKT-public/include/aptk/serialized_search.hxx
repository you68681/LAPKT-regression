/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SERIALIZED_SEARCH__
#define __SERIALIZED_SEARCH__

#include <aptk/search_prob.hxx>
#include <aptk/resources_control.hxx>
#include <aptk/closed_list.hxx>
//#include <aptk/iw.hxx>
#include <reachability.hxx>
#include <vector>
#include <algorithm>
#include <iostream>

namespace aptk {

namespace search {


template < typename Search_Model, typename Search_Strategy, typename Search_Node >
class Serialized_Search : public Search_Strategy {

public:

	typedef		typename Search_Model::State_Type		                          State;
	typedef 	Closed_List< Search_Node >			                          Closed_List_Type;

	Serialized_Search( 	const Search_Model& search_problem ) 
		: Search_Strategy( search_problem ), m_consistency_test(true), m_closed_goal_states( NULL )  {	   
		m_reachability = new aptk::agnostic::Reachability_Test(this->problem().task() );

	}

	virtual ~Serialized_Search() {
		delete m_reachability;
		m_closed_goal_states = NULL;
	}
	void                    set_consistency_test( bool b )           { m_consistency_test = b; }
	void            	set_closed_goal_states( Closed_List_Type* c ){ m_closed_goal_states = c; }
	void 			close_goal_state( Search_Node* n ) 	 { 
			if( closed_goal_states() ){
				//m_closed_goal_states->put( n ); 
				State* new_state = new State( this->problem().task() );
				new_state->set( n->state()->fluent_vec() );
				new_state->update_hash();				
				Search_Node* new_node = new Search_Node( new_state, n->action() );
				new_node->gn() = n->gn();
				m_closed_goal_states->put( new_node ); 
			}
	}
	Closed_List_Type*	closed_goal_states() 			 { return m_closed_goal_states; }


	void                    reset_closed_goal_states( ) { 
		if( closed_goal_states() ){
			// for ( typename Closed_List_Type::iterator i = m_closed_goal_states->begin();
			//       i != m_closed_goal_states->end(); i++ ) {
			// 	i->second = NULL;
			// }
			m_closed_goal_states->clear(); 
		}
	}


	bool 		        is_goal_state_closed( Search_Node* n ) 	 {
		if( !closed_goal_states() ) return false;
			       
		n->compare_only_state( true );
		Search_Node* n2 = this->closed_goal_states()->retrieve(n);
		n->compare_only_state( false );
		
		if ( n2 != NULL ) 
			return true;
		
		return false;
	}

	void debug_info( State*s, Fluent_Vec& unachieved ){
			
			std::cout << std::endl;
			std::cout << "Goals Achieved: ";


			for(Fluent_Vec::iterator it = m_goals_achieved.begin(); it != m_goals_achieved.end(); it++){
				std::cout << " " << this->problem().task().fluents()[ *it ]->signature(); 
			}
			
			std::cout << std::endl;
			std::cout << "Unachieved Goals: ";
			for(Fluent_Vec::iterator it = unachieved.begin(); it != unachieved.end(); it++){
				std::cout << " " << this->problem().task().fluents()[ *it ]->signature(); 
			}
			std::cout << std::endl;
			std::cout << "Current State: ";
			s->print( std::cout );

	}

	bool exclude_actions( Bit_Set& excluded, Fluent_Vec& persisting_goals ){

		bool changed = false;

        /**
         * If actions edel or adds fluent that has to persist, exclude action.
         */

        for (unsigned fl : persisting_goals) {
            for (auto const_a : this->problem().task().actions_adding(fl)) {
                unsigned idx = const_a->index();

                if (!excluded.isset(idx)) {
                    changed = true;
                    excluded.set(idx);

                }
            }

            for (auto const_a : this->problem().task().actions_edeleting(fl)) {
                unsigned idx = const_a->index();

                if (!excluded.isset(idx)) {
                    changed = true;
                    excluded.set(idx);
                }
            }

        }
		return changed;
	}


    bool check_exclude_new_actions( Bit_Set& excluded, unsigned persisting_goal ){

        bool changed = false;

        for (auto const_a : this->problem().task().actions_adding(persisting_goal)) {
            unsigned idx = const_a->index();

            if (!excluded.isset(idx)) {
                changed = true;
                break;
            }
        }

        if(!changed)
            for (auto const_a : this->problem().task().actions_edeleting(persisting_goal)) {
                unsigned idx = const_a->index();

                if (!excluded.isset(idx)) {
                    changed = true;
                    break;
                }
            }

        return changed;


    }

	virtual bool  is_goal( Search_Node* n ){

		const bool has_state = n->has_state();
		static Fluent_Vec added_fluents;
		static Fluent_Vec deleted_fluents;

		State* s = has_state ? n->state() : n->parent()->state();

		if( ! has_state ){
			added_fluents.clear();
			deleted_fluents.clear();
			n->parent()->state()->progress_lazy_state(  this->problem().task().actions()[ n->action() ], &added_fluents, &deleted_fluents );	
		}


		for(Fluent_Vec::iterator it =  m_goals_achieved.begin(); it != m_goals_achieved.end(); it++){
			if(  ! s->entails( *it ) ){
				if( ! has_state )
					n->parent()->state()->regress_lazy_state( this->problem().task().actions()[ n->action() ], &added_fluents, &deleted_fluents );

				return false;
			}
					
		}
		
		
		if( is_goal_state_closed( n ) ) 
			return false;
				
		bool new_goal_achieved = false; 
		Fluent_Vec unachieved;
        Fluent_Vec original_goal_candidates = m_goal_candidates;
		for(Fluent_Vec::iterator it = m_goal_candidates.begin(); it != m_goal_candidates.end(); it++){
			if(  s->entails( *it ) )
				{
                    Fluent_Vec persisting_goals;
					m_goals_achieved.push_back( *it );
                    persisting_goals = m_goals_achieved;

					if(!m_consistency_test){
						new_goal_achieved = true;
						continue;
					}

					auto search_excluded = m_goal_excluded_actions.find( persisting_goals );
					if ( search_excluded == m_goal_excluded_actions.end() ) {
                        auto new_item = m_goal_excluded_actions.emplace(persisting_goals, Bit_Set(this->problem().num_actions()));
                        exclude_actions( m_goal_excluded_actions[ persisting_goals ], persisting_goals );
                    }

					Bit_Set& excluded = m_goal_excluded_actions[ persisting_goals ];

                    bool new_excluded;
                    //Check if other goals true in current states persist along with the goal candidate
					do {
                        new_excluded = false;
                        for (auto g : original_goal_candidates) {

                            //If other goal is true in the state
                            if (*it == g) continue;
                            if (!s->entails(g)) continue;

                            //Check that all actions that can add the other goal are excluded
                            bool g_persists = true;
                            for (auto const_a : this->problem().task().actions_adding(g)) {
                                unsigned idx = const_a->index();

                                if (!excluded.isset(idx)) {
                                    g_persists = false;
                                    break;
                                }
                            }

                            //If true, then we also have to exclude actions conflicting with that goal as well
                            if (g_persists) {


                                persisting_goals.push_back(g);


                                bool excluded_g = check_exclude_new_actions(excluded, g);

                                #ifdef DEBUG
                                std::cout << "\t Implies goal "<< 	this->problem().task().fluents()[g]->signature() << std::endl;
                                for( auto const_a : this->problem().task().actions()) {
                                    Action a = *this->problem().task().actions()[const_a->index()];
                                    if (excluded.isset(a.index())) {
                                        std::cout << "\t" << a.signature() << std::endl;
                                    }
                                }
                                #endif

                                //Remove the persistent goal if it doesn't exclude any action. This means that the goal was already excluded
                                if (!excluded_g)
                                    persisting_goals.pop_back();
                                else{

                                    //create excluded persistent_goals if one doesn't exist yet
                                    auto search_excluded = m_goal_excluded_actions.find( persisting_goals );
                                    if ( search_excluded == m_goal_excluded_actions.end() ) {
                                        auto new_item = m_goal_excluded_actions.emplace(persisting_goals, Bit_Set(this->problem().num_actions()));
                                        exclude_actions( m_goal_excluded_actions[ persisting_goals ], persisting_goals );
                                    }

                                    //point to excluded set to new persistent goals set
                                    excluded = m_goal_excluded_actions[ persisting_goals ];
                                }

                                new_excluded = new_excluded || excluded_g;


                            }
                        }
                    }while (new_excluded);

					#ifdef DEBUG
						if ( this->verbose() )
							debug_info( s, unachieved );

					std::cout << "Excluded actions for goal "<< 	this->problem().task().fluents()[*it]->signature() << std::endl;
                    for( auto const_a : this->problem().task().actions()) {
                        Action a = *this->problem().task().actions()[const_a->index()];
                        if (excluded.isset(a.index())) {
                            std::cout << a.signature() << std::endl;
                        }
                    }
                    #endif

					if(m_reachability->is_reachable( s->fluent_vec() , this->problem().goal() , excluded  ) ) {
                        std::ofstream h2_stream;
                        h2_stream.open("action_record.txt",std::ios::app);
                        h2_stream<<"\t New Consistent Goal "<<this->problem().task().fluents()[ *it ]->signature() <<"\n";
                        std::cout << "\t New Consistent Goal " << this->problem().task().fluents()[ *it ]->signature()<<std::endl;
                        new_goal_achieved = true;
                        h2_stream.close();
                    }
					else{	
						unachieved.push_back( *it );
						m_goals_achieved.pop_back();
					}
					
				}
			else
				unachieved.push_back( *it );
		}

		if( ! has_state )
			n->parent()->state()->regress_lazy_state( this->problem().task().actions()[ n->action() ], &added_fluents, &deleted_fluents );

		if ( new_goal_achieved ){
			m_goal_candidates = unachieved;				

			if( ! has_state )
//				n->set_state( n->parent()->state()->progress_through( *(this->problem().task().actions()[ n->action() ]) ));
                n->set_state( n->parent()->state()->regress_through( *(this->problem().task().actions()[ n->action() ]) ));

			close_goal_state( n );
			return true;
		}
		else
			return false;	

	}
	
	virtual bool	find_solution( float& cost, std::vector<Action_Idx>& plan ) {
		
		Search_Node* end = NULL;
		State* new_init_state = NULL;
		m_goals_achieved.clear();
		m_goal_candidates.clear();
		m_goal_candidates.insert( m_goal_candidates.begin(), 
					  this->problem().goal().begin(), this->problem().goal().end() );

		do{
			end = this->do_search();		

			if ( end == NULL ) return false;

			std::vector<Action_Idx> partial_plan;
			this->extract_plan( this->m_root, end, partial_plan, cost );	
			plan.insert( plan.end(), partial_plan.begin(), partial_plan.end() );			

			new_init_state = new State( this->problem().task() );
			new_init_state->set( end->state()->fluent_vec() );
			new_init_state->update_hash();
			this->start( new_init_state );


		}while( !this->problem().goal( *new_init_state ) );


		
		return true;
	}

	
	Fluent_Vec&        goal_candidates(){ return m_goal_candidates; }
	Fluent_Vec&        goals_achieved(){ return m_goals_achieved; }
	
protected:	       
	aptk::agnostic::Reachability_Test*      m_reachability;
    Fluent_Vec ::iterator                   vector_iterator;
	Fluent_Vec                              m_goals_achieved;
	Fluent_Vec                              m_goal_candidates;
    Fluent_Vec                              m_goal_h2;
	bool                                    m_consistency_test;
	Closed_List_Type*			m_closed_goal_states;
    float                    h_val;

    //Data Structures for the computation of exclude_actions

    template <typename Container> // we can make this generic for any container [1]
    struct container_hash {
        std::size_t operator()(Container const& c) const {
            Hash_Key hasher;
            hasher.add( c );
            return (size_t)hasher;
        }
    };

    std::unordered_map<Fluent_Vec ,Bit_Set, container_hash<Fluent_Vec>> m_goal_excluded_actions;
};

}

}



#endif // serialized_search.hxx
