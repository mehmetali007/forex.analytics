#include "nan.h"

#include "../include/TradingSimulator.h"
#include <iostream>


double Trade::getRevenue() const {
    return this->Buy ?
           this->End->Close - this->Start->Close :
           this->Start->Close - this->End->Close;
}

void Trade::ToArray(
    const std::vector<Trade>& trades,
    v8::Local<v8::Array>& output) {

    size_t tradeCount = trades.size();

    for (size_t i = 0; i < tradeCount; i++) {

        v8::Local<v8::Object> object = Nan::New<v8::Object>();

        const Trade& trade = trades[i];

        Trade::ToObject(trade, object);

        output->Set(static_cast<uint32_t>(i), object);
    }
}

void Trade::ToObject(
    const Trade& trade,
    v8::Local<v8::Object>& output) {

    output->Set(Nan::New<v8::String>("Buy").ToLocalChecked(),
                Nan::New<v8::Boolean>(trade.Buy));

    output->Set(Nan::New<v8::String>("Revenue").ToLocalChecked(),
                Nan::New<v8::Number>(trade.getRevenue()));

    output->Set(Nan::New<v8::String>("MaximumLoss").ToLocalChecked(),
                Nan::New<v8::Number>(trade.MaximumLoss));

    output->Set(Nan::New<v8::String>("MaximumProffit").ToLocalChecked(),
                Nan::New<v8::Number>(trade.MaximumProffit));

	v8::Local<v8::Object> start = Nan::New<v8::Object>();
    v8::Local<v8::Object> end = Nan::New<v8::Object>();

    Candlestick::ToObject(*trade.Start, start);
    Candlestick::ToObject(*trade.End, end);

    output->Set(Nan::New<v8::String>("start").ToLocalChecked() ,start);
    output->Set(Nan::New<v8::String>("end").ToLocalChecked(), end);
}

std::vector<Trade>* TradingSimulator::Simulate(
    const BinaryTreeChromosome * chromosome,
    const std::vector<IndicatorTuple>* data) const {

    std::vector<Trade>* trades = new std::vector<Trade>();

    const Candlestick * start = nullptr;

	int proffitIndex = 0;
	int lossIndex = 0;
    double maximumPotentialLoss = 0.0;
    double maximumPotentialProffit = 0.0;


		bool shouldBuy, shouldSell, buy = false;

		for (unsigned long i = 0; i < data->size(); i++) {
			shouldBuy = chromosome->shouldBuy(data->at(i).Element);
			shouldSell = chromosome->shouldSell(data->at(i).Element);

			if (shouldBuy == true && shouldSell == false) {
				// if we're in trade and we're selling then close it and start a new one
				if (start != nullptr && buy == false) {

					Trade t;
					t.Start = start;
					t.End = &data->at(i).Element.begin()->second.candlestick;
					t.Buy = buy;
					t.MaximumProffit = maximumPotentialProffit;
					t.MaximumLoss = maximumPotentialLoss;
					t.ProffitBeforeLoss = proffitIndex < lossIndex;

					trades->push_back(t);

					start = &(data->at(i).Element.begin()->second.candlestick);

					proffitIndex = 0;
					lossIndex = 0;
					maximumPotentialLoss = 0.0;
					maximumPotentialProffit = 0.0;
				}
				// or if not in trade at all
				else if (start == nullptr) {
					start = &(data->at(i).Element.begin()->second.candlestick);
				}


				buy = true;
			}

			if (buy == true && start != nullptr)
			{
				double intermediateProffit = data->at(i).Element.begin()->second.candlestick.High - start->Close;
				double intermediateLoss = start->Close - data->at(i).Element.begin()->second.candlestick.Low;

				if (intermediateLoss > maximumPotentialLoss) {
					maximumPotentialLoss = intermediateLoss;
					lossIndex = i;
				}

				if (intermediateProffit > maximumPotentialProffit) {
					maximumPotentialProffit = intermediateProffit;
					proffitIndex = i;
				}
			}

			if (shouldBuy == false && shouldSell == true) {
				if (start != nullptr && buy == true) {

					Trade t;
					t.Start = start;
					t.End = &data->at(i).Element.begin()->second.candlestick;
					t.Buy = buy;
					t.MaximumProffit = maximumPotentialProffit;
					t.MaximumLoss = maximumPotentialLoss;
					t.ProffitBeforeLoss = proffitIndex < lossIndex;

					trades->push_back(t);

					start = &(data->at(i).Element.begin()->second.candlestick);

					proffitIndex = 0;
					lossIndex = 0;
					maximumPotentialLoss = 0.0;
					maximumPotentialProffit = 0.0;

				}
				else if (start == nullptr) {
					start = &(data->at(i).Element.begin()->second.candlestick);
				}

				buy = false;
			}

			if (buy == false && start != nullptr)
			{

				double intermediateProffit = start->Close - data->at(i).Element.begin()->second.candlestick.Low;
				double intermediateLoss = data->at(i).Element.begin()->second.candlestick.High - start->Close;

				if (intermediateLoss > maximumPotentialLoss) {
					maximumPotentialLoss = intermediateLoss;
					lossIndex = i;
				}

				if (intermediateProffit > maximumPotentialProffit) {
					maximumPotentialProffit = intermediateProffit;
					proffitIndex = i;
				}
			}

    }

    return trades;
} // TradingSimulator::Simulate
